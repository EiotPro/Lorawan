#include <Arduino.h>
#include "config.h"
#include "utils.h"
#include "wifi_manager.h"
#include "ota_manager.h"
#include <LittleFS.h>

// --- LoRaWAN ABP Configuration ---
const char* DevAdd = LORAWAN_DEV_ADDR;  // Device Address from ChirpStack
const char* NetKey = LORAWAN_NWKS_KEY;  // Network Session Key from ChirpStack
const char* SessKey = LORAWAN_APPS_KEY; // Application Session Key from ChirpStack

// Timers for various operations
unsigned long lastLoRaTransmission = 0;

// Function prototypes
float readCurrent();
bool waitForModuleReady();
bool initializeLoRaWAN();
bool sendLoRaWANPayload(float currentValue);
bool listenForDownlink();
void processDownlinkPayload(String payload);
void handleOTACommand();
void checkForPendingDownlinks();
bool checkForDownlinkData();
bool processUARTLine(String line);
void testDownlinkSystem();
void checkForPendingOTAUpdate();

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
  log_info("WCS6800 Current LoRaWAN Monitor");
  log_format(LOG_INFO, "Device: %s", DEVICE_NAME);
  log_format(LOG_INFO, "Version: %s", DEVICE_VERSION);
  log_format(LOG_INFO, "Build: %s", BUILD_TIMESTAMP);

  // Check for pending OTA updates
  checkForPendingOTAUpdate();
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

  // Run downlink system test
  delay(2000); // Wait a bit for everything to settle
  testDownlinkSystem();
}

void loop() {
  // Handle WiFi events
  if (WIFI_ENABLED) {
    handleWiFiEvents();
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
  

  

  
  // Check for any pending downlinks (Class C continuous listening)
  static unsigned long lastDownlinkCheck = 0;
  if (currentTime - lastDownlinkCheck > 3000) { // Check every 3 seconds
    checkForPendingDownlinks();
    lastDownlinkCheck = currentTime;
  }

  // Also check for any immediate UART data that might be downlink events
  if (Serial1.available()) {
    String line = "";
    unsigned long lineStart = millis();

    // Read a complete line with timeout
    while (millis() - lineStart < 1000) {
      if (Serial1.available()) {
        char c = Serial1.read();
        if (c == '\n' || c == '\r') {
          if (line.length() > 0) {
            processUARTLine(line);
            break;
          }
        } else {
          line += c;
        }
      }
      delay(10);
    }
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
    log_info("LoRaWAN network join command sent successfully!");

    // Wait for join confirmation
    delay(2000);
    unsigned long joinStartTime = millis();
    bool joinSuccess = false;

    while (millis() - joinStartTime < 30000) { // 30 second timeout for join
      if (Serial1.available()) {
        String joinResponse = Serial1.readString();
        joinResponse.trim();
        log_format(LOG_DEBUG, "Join response: %s", joinResponse.c_str());

        if (joinResponse.indexOf("+EVT:JOINED") != -1) {
          log_info("Successfully joined LoRaWAN network!");
          joinSuccess = true;
          break;
        } else if (joinResponse.indexOf("+EVT:JOIN FAILED") != -1) {
          log_error("Failed to join LoRaWAN network");
          return false;
        }
      }
      delay(500);
    }

    if (!joinSuccess) {
      log_error("Join timeout - no confirmation received");
      return false;
    }

    // Verify Class C is active
    delay(1000);
    if (sendATCommand("AT+CLASS=?", "C", 3000)) {
      log_info("Class C confirmed active");
    } else {
      log_warning("Class C verification failed, but continuing");
    }

    log_info("LoRaWAN initialized and joined successfully!");
    return true;
  } else {
    log_error("ERROR: Failed to send join command");
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

  // First, immediately check for any pending downlinks
  if (checkForDownlinkData()) {
    return true;
  }

  // Then listen for new downlink events
  unsigned long startTime = millis();
  unsigned long timeout = 15000; // 15 second timeout
  String buffer = "";

  while (millis() - startTime < timeout) {
    // Read character by character to avoid missing data
    while (Serial1.available()) {
      char c = Serial1.read();
      buffer += c;

      // Check for complete lines ending with \n or \r
      if (c == '\n' || c == '\r') {
        if (buffer.length() > 2) {
          buffer.trim();
          log_format(LOG_DEBUG, "UART line: '%s'", buffer.c_str());

          // Process the complete line
          if (processUARTLine(buffer)) {
            return true; // Found and processed downlink
          }
        }
        buffer = ""; // Reset buffer for next line
      }

      // Prevent buffer overflow
      if (buffer.length() > 200) {
        log_warning("UART buffer overflow, clearing");
        buffer = "";
      }
    }

    delay(50); // Small delay to prevent tight loop
  }

  log_info("No downlink received within timeout period");
  return false;
}

// Function to process received downlink payload
void processDownlinkPayload(String payload) {
  payload.trim();
  payload.toUpperCase();

  log_format(LOG_INFO, "Processing downlink payload: %s", payload.c_str());

  if (payload.length() == 0) {
    log_warning("Empty payload received");
    return;
  }

  // Process single byte commands
  if (payload.length() == 2) { // Single byte in hex
    if (payload == "01") {
      log_info("LED ON command received");
      digitalWrite(LED_PIN, HIGH);
    }
    else if (payload == "02") {
      log_info("LED OFF command received");
      digitalWrite(LED_PIN, LOW);
    }
    else if (payload == "03") {
      log_info("OTA UPDATE command received");
      if (OTA_ENABLED) {
        handleOTACommand();
      } else {
        log_warning("OTA not enabled, ignoring command");
      }
    }
    else if (payload == "04") {
      log_info("LED BLINK command received");
      // Blink LED 5 times
      for (int i = 0; i < 5; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
      }
    }
    else {
      log_format(LOG_WARNING, "Unknown command: %s", payload.c_str());
    }
  }
  else {
    log_format(LOG_INFO, "Multi-byte payload received: %s", payload.c_str());
    // Add processing for longer payloads if needed
  }
}

// Function to handle OTA command from downlink
void handleOTACommand() {
  log_info("Handling OTA update command from downlink");
  if (OTA_ENABLED) {
    triggerOTAUpdate(OTA_METHOD_WIFI);
  } else {
    log_error("OTA is not enabled in configuration");
  }
}

// Function to check for pending downlinks (for Class C)
void checkForPendingDownlinks() {
  checkForDownlinkData();
}

// Function to check for downlink data using AT+RECV
bool checkForDownlinkData() {
  log_debug("Checking for pending downlink data...");

  // Clear buffer first
  clearUartBuffer();

  // Send AT+RECV=? command
  Serial1.print("AT+RECV=?\r\n");
  delay(100); // Give module time to respond

  unsigned long startTime = millis();
  String response = "";
  bool foundOK = false;

  // Read response with timeout
  while ((millis() - startTime) < 3000) {
    if (Serial1.available()) {
      char c = Serial1.read();
      response += c;

      // Check for command completion
      if (response.indexOf("OK") != -1) {
        foundOK = true;
        break;
      }
      if (response.indexOf("ERROR") != -1) {
        log_debug("AT+RECV returned ERROR");
        return false;
      }
    }
    delay(10);
  }

  if (!foundOK) {
    log_debug("AT+RECV timeout");
    return false;
  }

  response.trim();
  log_format(LOG_DEBUG, "AT+RECV full response: '%s'", response.c_str());

  // Look for port:payload pattern before OK
  int okPos = response.indexOf("OK");
  if (okPos > 0) {
    String dataLine = response.substring(0, okPos);
    dataLine.trim();

    // Check if we have data (not just "0:" which means no data)
    if (dataLine.length() > 2 && !dataLine.startsWith("0:")) {
      log_format(LOG_INFO, "Found downlink data: %s", dataLine.c_str());

      // Parse port:payload
      int colonPos = dataLine.indexOf(":");
      if (colonPos != -1 && colonPos < dataLine.length() - 1) {
        String port = dataLine.substring(0, colonPos);
        String payload = dataLine.substring(colonPos + 1);
        payload.trim();

        if (payload.length() > 0) {
          log_format(LOG_INFO, "Processing downlink from port %s: %s", port.c_str(), payload.c_str());
          processDownlinkPayload(payload);
          return true;
        }
      }
    }
  }

  return false;
}

// Function to process individual UART lines for downlink events
bool processUARTLine(String line) {
  line.trim();

  if (line.length() == 0) {
    return false;
  }

  // Check for downlink event patterns
  if (line.indexOf("+EVT:RX") != -1) {
    log_format(LOG_INFO, "Downlink event detected: %s", line.c_str());

    // Extract RSSI and SNR if present
    if (line.indexOf("RSSI") != -1) {
      int rssiStart = line.indexOf("RSSI ") + 5;
      int rssiEnd = line.indexOf(",", rssiStart);
      if (rssiEnd == -1) rssiEnd = line.indexOf(" ", rssiStart);
      if (rssiEnd != -1) {
        String rssiStr = line.substring(rssiStart, rssiEnd);
        log_format(LOG_INFO, "Downlink RSSI: %s dBm", rssiStr.c_str());
      }
    }

    if (line.indexOf("SNR") != -1) {
      int snrStart = line.indexOf("SNR ") + 4;
      String snrStr = line.substring(snrStart);
      snrStr.trim();
      log_format(LOG_INFO, "Downlink SNR: %s", snrStr.c_str());
    }

    // Wait a moment then check for data
    delay(200);
    return checkForDownlinkData();
  }

  // Check for direct port:payload format
  if (line.indexOf(":") != -1 && line.length() > 2) {
    int colonPos = line.indexOf(":");
    String beforeColon = line.substring(0, colonPos);
    String afterColon = line.substring(colonPos + 1);

    // Check if it looks like port:payload (port should be 1-3 digits)
    if (beforeColon.length() <= 3 && beforeColon.toInt() > 0 && afterColon.length() > 0) {
      uint8_t fport = beforeColon.toInt();
      log_format(LOG_INFO, "Direct downlink detected on port %d: %s", fport, afterColon.c_str());

      // Check if this is a FUOTA message (disabled for now)
      // if (fport >= 200 && fport <= 202) {
      //   return processFuotaDownlink(fport, afterColon);
      // } else {
        processDownlinkPayload(afterColon);
        return true;
      // }
    }
  }

  return false;
}

// Test function to verify downlink system
void testDownlinkSystem() {
  log_info("=== Testing Downlink System ===");

  // Test 1: Check AT+RECV command
  log_info("Test 1: Checking AT+RECV command...");
  if (checkForDownlinkData()) {
    log_info("✓ Found pending downlink data");
  } else {
    log_info("✓ No pending downlink data (normal)");
  }

  // Test 2: Verify Class C mode
  log_info("Test 2: Verifying Class C mode...");
  if (sendATCommand("AT+CLASS=?", "C", 3000)) {
    log_info("✓ Class C mode confirmed");
  } else {
    log_warning("✗ Class C mode not confirmed");
  }

  // Test 3: Check join status
  log_info("Test 3: Checking join status...");
  if (sendATCommand("AT+NJS=?", "1", 3000)) {
    log_info("✓ Device is joined to network");
  } else {
    log_warning("✗ Device not joined to network");
  }

  // Test 4: Test LED functionality
  log_info("Test 4: Testing LED functionality...");
  log_info("Testing LED ON...");
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  log_info("Testing LED OFF...");
  digitalWrite(LED_PIN, LOW);
  delay(500);
  log_info("✓ LED test completed");

  log_info("=== Downlink System Test Complete ===");
  log_info("To test downlinks:");
  log_info("1. Send '01' from ChirpStack to turn LED ON");
  log_info("2. Send '02' from ChirpStack to turn LED OFF");
  log_info("3. Send '04' from ChirpStack for LED blink test");
}

// Function to process FUOTA downlink messages (disabled for now)
/*
bool processFuotaDownlink(uint8_t fport, String hexPayload) {
  log_format(LOG_INFO, "Processing FUOTA message on port %d: %s", fport, hexPayload.c_str());

  // Convert hex string to bytes
  int payloadLength = hexPayload.length() / 2;
  if (payloadLength == 0 || hexPayload.length() % 2 != 0) {
    log_error("Invalid FUOTA hex payload length");
    return false;
  }

  uint8_t* payload = (uint8_t*)malloc(payloadLength);
  if (!payload) {
    log_error("Failed to allocate memory for FUOTA payload");
    return false;
  }

  // Convert hex string to bytes
  for (int i = 0; i < payloadLength; i++) {
    String byteStr = hexPayload.substring(i * 2, i * 2 + 2);
    payload[i] = (uint8_t)strtol(byteStr.c_str(), NULL, 16);
  }

  // Process FUOTA message
  bool result = fuotaHandler.handleFuotaMessage(fport, payload, payloadLength);

  free(payload);
  return result;
}
*/

// Check for pending OTA updates from previous session
void checkForPendingOTAUpdate() {
  if (!LittleFS.begin()) {
    log_warning("LittleFS not available for OTA check");
    return;
  }

  // Check for OTA metadata file
  File metadataFile = LittleFS.open("/ota_metadata.json", "r");
  if (metadataFile) {
    log_info("🔍 Pending OTA update detected!");

    // Read metadata
    String metadata = metadataFile.readString();
    metadataFile.close();

    log_info("📋 OTA Metadata:");
    log_format(LOG_INFO, "%s", metadata.c_str());

    // Check if firmware file still exists
    File firmwareFile = LittleFS.open("/firmware.bin", "r");
    if (firmwareFile) {
      size_t firmwareSize = firmwareFile.size();
      log_info("✅ Downloaded firmware file verified!");
      log_format(LOG_INFO, "📁 Firmware available: %d bytes at /firmware.bin", firmwareSize);
      log_warning("⚠️  Manual intervention required for flash update");
      log_info("💡 For production: Implement bootloader integration");

      // Show file contents for verification
      log_info("📄 Available OTA files:");
      File root = LittleFS.open("/", "r");
      if (root && root.isDirectory()) {
        File file = root.openNextFile();
        while (file) {
          if (!file.isDirectory()) {
            log_format(LOG_INFO, "  • %s (%d bytes)", file.name(), file.size());
          }
          file = root.openNextFile();
        }
      }

      firmwareFile.close();
    } else {
      log_warning("⚠️  Downloaded firmware file not found");
    }

    // Mark metadata as processed (but keep files for manual inspection)
    File processedFile = LittleFS.open("/ota_processed.txt", "w");
    if (processedFile) {
      processedFile.printf("OTA processed at boot: %lu ms\n", millis());
      processedFile.close();
    }

    log_info("🧹 OTA metadata processed (files preserved for inspection)");
  } else {
    log_info("ℹ️  No pending OTA updates");
  }
}
