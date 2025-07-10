#include <Arduino.h>
#include "config.h"
#include "utils.h"
#include "wifi_manager.h"
#include "ble_manager.h"
#include "ota_manager.h"

// --- LoRaWAN ABP Configuration ---
const char* DevAdd = LORAWAN_DEV_ADDR;  // Device Address from ChirpStack
const char* NetKey = LORAWAN_NWKS_KEY;  // Network Session Key from ChirpStack
const char* SessKey = LORAWAN_APPS_KEY; // Application Session Key from ChirpStack

// Timers for various operations
unsigned long lastMQTTPublish = 0;
unsigned long lastBLEUpdate = 0;
unsigned long lastLoRaTransmission = 0;

// Function prototypes
float readCurrent();
bool waitForModuleReady();
bool initializeLoRaWAN();
bool sendLoRaWANPayload(float currentValue);
bool listenForDownlink();

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait for Serial for up to 3 seconds
  
  // Initialize Serial1 with specified baud rate
  Serial1.begin(UART_BAUDRATE);
  
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, HIGH);
  
  // Initialize ADC
  analogReadResolution(12); // Set ADC resolution to 12-bit
  
  log_info("==========================================================");
  log_info("WCS6800 Current Sensor LoRaWAN Monitor");
  log_info("==========================================================");
  log_format(LOG_INFO, "UART: TX=GP%d, RX=GP%d @ %d baud", UART_TX_PIN, UART_RX_PIN, UART_BAUDRATE);
  log_format(LOG_INFO, "ADC: WCS6800 on GP%d", ADC_PIN);
  log_format(LOG_INFO, "Device Address: %s", DevAdd);
  log_info("==========================================================");
  
  // Run diagnostics
  if (!runFullDiagnostics()) {
    log_error("Hardware diagnostics failed, continuing with caution");
  }
  
  // Initialize WiFi if enabled
  if (WIFI_ENABLED) {
    if (setupWiFi()) {
      log_info("WiFi initialized successfully");
    } else {
      log_error("WiFi initialization failed");
    }
  }
  
  // Initialize BLE if enabled
  if (BLE_ENABLED) {
    if (setupBLE()) {
      log_info("BLE initialized successfully");
    } else {
      log_error("BLE initialization not available on this hardware");
    }
  }
  
  // Initialize OTA if enabled
  if (OTA_ENABLED) {
    if (setupOTA()) {
      log_info("OTA update system initialized successfully");
    } else {
      log_error("OTA initialization failed");
    }
  }
  
  // Initialize LoRaWAN
  if (!initializeLoRaWAN()) {
    log_error("FATAL ERROR: LoRaWAN initialization failed");
    handleError(ERR_LORAWAN_INIT);
    return;
  }
  
  log_info("Starting main monitoring loop...");
  lastLoRaTransmission = millis(); // Initialize the last transmission time
}

void loop() {
  // Handle WiFi events
  if (WIFI_ENABLED) {
    handleWiFiEvents();
  }
  
  // Handle BLE events
  if (BLE_ENABLED) {
    handleBLEEvents();
  }
  
  // Handle OTA events
  if (OTA_ENABLED) {
    handleOTAEvents();
    
    // If OTA is in progress, skip the regular monitoring
    if (getOTAStatus() != OTA_STATUS_IDLE) {
      delay(100);
      return;
    }
  }
  
  // Read current from sensor
  float currentVal = readCurrent();
  
  // Check if it's time to transmit via LoRaWAN
  unsigned long currentTime = millis();
  if (currentTime - lastLoRaTransmission >= TX_INTERVAL) {
    log_format(LOG_INFO, "\nCurrent Reading: %.3f A", currentVal);
    
    // Format data as JSON for dashboard integration
    String jsonData = formatCurrentAsJson(currentVal);
    
    // Update BLE characteristic if connected and enough time has passed
    if (BLE_ENABLED && (currentTime - lastBLEUpdate > BLE_TX_INTERVAL)) {
      updateBLECurrentValue(currentVal);
      lastBLEUpdate = currentTime;
    }
    
    // Publish to MQTT if enabled and enough time has passed
    if (WIFI_ENABLED && MQTT_ENABLED && (currentTime - lastMQTTPublish > WIFI_TX_INTERVAL)) {
      publishCurrentData(currentVal);
      lastMQTTPublish = currentTime;
    }
    
    // Send data via LoRaWAN
    if (sendLoRaWANPayload(currentVal)) {
      // Listen for downlink commands
      listenForDownlink();
    } else {
      log_info("Skipping downlink listen due to send failure");
      handleError(ERR_LORAWAN_SEND);
    }
    
    lastLoRaTransmission = currentTime;
  }
  
  // Process more frequent updates for WiFi/BLE without LoRaWAN transmissions
  // to avoid overloading the network
  if (WIFI_ENABLED && MQTT_ENABLED && (currentTime - lastMQTTPublish > WIFI_TX_INTERVAL)) {
    publishCurrentData(currentVal);
    lastMQTTPublish = currentTime;
  }
  
  if (BLE_ENABLED && (currentTime - lastBLEUpdate > BLE_TX_INTERVAL)) {
    updateBLECurrentValue(currentVal);
    lastBLEUpdate = currentTime;
  }
  
  // Short delay to prevent tight loop
  delay(100);
}

float readCurrent() {
  // Read raw ADC value (0-4095 for 12-bit)
  int adcValue = analogRead(ADC_PIN);
  
  // Convert ADC value to voltage
  float voltage = (float(adcValue) / ADC_MAX_VALUE) * ADC_REF_VOLTAGE;
  
  // Calculate current based on WCS6800 characteristics
  // Current = (Voltage_measured - Offset_voltage) / Sensitivity
  float current = (voltage - WCS6800_OFFSET_VOLTAGE) / WCS6800_SENSITIVITY;
  
  return current;
}

bool waitForModuleReady() {
  log_info("Waiting for RAK3172 module to be ready...");
  
  // Clear any boot messages
  delay(3000);
  clearUartBuffer();
  
  // Try to get a response to basic AT command
  int maxAttempts = 5;
  for (int attempt = 0; attempt < maxAttempts; attempt++) {
    log_format(LOG_INFO, "Attempt %d to communicate with module...", attempt + 1);
    
    if (sendATCommand("AT", "OK", 3000)) {
      log_info("Module is ready!");
      return true;
    }
    
    delay(1000);
  }
  
  log_error("Failed to communicate with module after multiple attempts");
  return false;
}

bool initializeLoRaWAN() {
  log_info("Initializing LoRaWAN ABP mode...");
  
  // Wait for module to be ready
  if (!waitForModuleReady()) {
    log_error("ERROR: Module not responding to AT commands");
    return false;
  }
  
  // Configure LoRaWAN parameters
  struct {
    const char* command;
    const char* description;
  } commands[] = {
    {"AT+NJM=0", "ABP mode"},
    {"AT+CLASS=C", "Class C"},
    {"AT+BAND=3", "IN865 region"},
    {"", "Device address"},  // Will be filled dynamically
    {"", "App session key"}, // Will be filled dynamically
    {"", "Network session key"} // Will be filled dynamically
  };
  
  // Set device address
  String devAddrCmd = "AT+DEVADDR=" + String(DevAdd);
  String appSKeyCmd = "AT+APPSKEY=" + String(SessKey);
  String nwkSKeyCmd = "AT+NWKSKEY=" + String(NetKey);
  
  // Execute basic commands
  for (int i = 0; i < 3; i++) {
    log_format(LOG_INFO, "Setting %s...", commands[i].description);
    if (!sendATCommand(commands[i].command, "OK", 3000)) {
      log_format(LOG_ERROR, "ERROR: Failed to set %s", commands[i].description);
      return false;
    }
    delay(500);
  }
  
  // Set device address
  log_info("Setting device address...");
  if (!sendATCommand(devAddrCmd.c_str(), "OK", 3000)) {
    log_error("ERROR: Failed to set device address");
    return false;
  }
  delay(500);
  
  // Set app session key
  log_info("Setting app session key...");
  if (!sendATCommand(appSKeyCmd.c_str(), "OK", 3000)) {
    log_error("ERROR: Failed to set app session key");
    return false;
  }
  delay(500);
  
  // Set network session key
  log_info("Setting network session key...");
  if (!sendATCommand(nwkSKeyCmd.c_str(), "OK", 3000)) {
    log_error("ERROR: Failed to set network session key");
    return false;
  }
  delay(500);
  
  // Join the network
  log_info("Joining LoRaWAN network...");
  if (sendATCommand("AT+JOIN", "OK", 10000)) {
    log_info("LoRaWAN initialized and joined successfully!");
    return true;
  } else {
    log_error("ERROR: Failed to join LoRaWAN network");
    return false;
  }
}

bool sendLoRaWANPayload(float currentValue) {
  // Convert float current_value to a compact binary payload.
  // Use a 2-byte signed integer for current in milliAmps (mA).
  int32_t currentMA_temp = int32_t(currentValue * 1000); // Convert Amps to milliAmps
  
  // Ensure value fits within int16_t range
  if (currentMA_temp > 32767) currentMA_temp = 32767;
  if (currentMA_temp < -32768) currentMA_temp = -32768;
  
  // Now convert to int16_t for the payload
  int16_t currentMA = (int16_t)currentMA_temp;
  
  // Pack the integer into 2 bytes (big-endian)
  uint8_t payloadBytes[2];
  payloadBytes[0] = (currentMA >> 8) & 0xFF;  // Most Significant Byte
  payloadBytes[1] = currentMA & 0xFF;         // Least Significant Byte
  
  // Convert bytes to hex string for AT command
  String hexPayload = "";
  for (int i = 0; i < 2; i++) {
    if (payloadBytes[i] < 16) hexPayload += "0";
    hexPayload += String(payloadBytes[i], HEX);
  }
  hexPayload.toUpperCase();
  
  // Send LoRaWAN uplink using AT+SEND command
  String command = "AT+SEND=2:" + hexPayload;
  log_format(LOG_INFO, "Sending payload: %s (Current: %.3f A)", hexPayload.c_str(), currentValue);
  
  if (sendATCommand(command.c_str(), "OK", 10000)) {
    log_info("Payload sent successfully");
    return true;
  } else {
    log_error("ERROR: Failed to send payload");
    return false;
  }
}

bool listenForDownlink() {
  log_info("Listening for downlink messages...");
  int timeoutCount = 0;
  int maxTimeout = 15000; // Wait up to 15 seconds for downlink
  
  while (timeoutCount < maxTimeout) {
    if (Serial1.available()) {
      String downlinkData = Serial1.readString();
      downlinkData.trim();
      log_format(LOG_INFO, "Received: %s", downlinkData.c_str());
      
      // Check for transmission done event
      if (downlinkData.indexOf("+EVT:TX_DONE") != -1) {
        log_info("Uplink transmission confirmed");
      }
      
      // Check for downlink event
      else if (downlinkData.indexOf("+EVT:RX_C") != -1 || downlinkData.indexOf("+EVT:RX_") != -1) {
        // Extract payload (last part after colons)
        int lastColonIndex = downlinkData.lastIndexOf(':');
        if (lastColonIndex != -1) {
          String payload = downlinkData.substring(lastColonIndex + 1);
          payload.trim();
          log_format(LOG_INFO, "Downlink payload: %s", payload.c_str());
          
          // Process commands
          if (payload == "01") {
            log_info("LED ON command received");
            digitalWrite(LED_PIN, HIGH);
          } else if (payload == "02") {
            log_info("LED OFF command received");
            digitalWrite(LED_PIN, LOW);
          } else if (payload == "03") {
            log_info("OTA UPDATE command received");
            if (OTA_ENABLED && WIFI_ENABLED) {
              log_info("Triggering WiFi OTA update");
              triggerOTAUpdate(OTA_METHOD_WIFI);
            } else {
              log_error("OTA updates are disabled in config or WiFi is not available");
            }
          } else if (payload == "04") {
            log_info("LED BLINK command received");
            for (int i = 0; i < 3; i++) { // Blink 3 times
              digitalWrite(LED_PIN, HIGH);
              delay(300);
              digitalWrite(LED_PIN, LOW);
              delay(300);
            }
          } else {
            log_format(LOG_INFO, "Unknown command: %s", payload.c_str());
          }
        }
        return true;
      }
    }
    
    delay(100);
    timeoutCount += 100;
  }
  
  log_info("No downlink received within timeout period");
  return false;
} 