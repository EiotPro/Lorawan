#include <Arduino.h>
#include <SoftwareSerial.h>

// --- WCS6800 Sensor Configuration ---
// Connect WCS6800 VOUT to RPi Pico ADC pin (e.g., GP26)
// Connect WCS6800 VCC to RPi Pico 3.3V
// Connect WCS6800 GND to RPi Pico GND

// ADC pin for WCS6800 output (GP26 corresponds to ADC0)
#define ADC_PIN 26 // GPIO26 / ADC0

// RPi Pico ADC has 12-bit resolution (0-4095)
#define ADC_MAX_VALUE 4095.0

// RPi Pico ADC reference voltage is 3.3V
#define ADC_REF_VOLTAGE 3.3 // Volts

// WCS6800 specifications (when powered by 3.3V)
// Sensitivity: 42.9 mV/A (0.0429 V/A)
// Output at 0A: 1.65V
#define WCS6800_SENSITIVITY 0.0429 // V/A
#define WCS6800_OFFSET_VOLTAGE 1.65 // Volts (Output at 0A current)

// --- RAK3172 UART Communication Configuration ---
// RPi Pico UART0 is connected to RAK3172 UART2 (as per schematic)
// TX: GP0, RX: GP1
#define UART_TX_PIN 0 // GP0 (connected to RAK3172 RX)
#define UART_RX_PIN 1 // GP1 (connected to RAK3172 TX)
#define UART_BAUDRATE 115200

// --- LoRaWAN ABP Configuration ---
// Replace with your actual ABP credentials from ChirpStack
const char* DevAdd = "01d3257c";  // Device Address from ChirpStack
const char* NetKey = "06ebd62a3b4e2ed8d45d38d0f515988e";  // Network Session Key from ChirpStack
const char* SessKey = "ef54ccd9b3d974e8736c60d916ad6e96";  // Application Session Key from ChirpStack

// Setup LED and reset pin
#define LED_PIN 25
#define RST_PIN 30

// UART object for RAK3172 communication
SoftwareSerial rakSerial(UART_RX_PIN, UART_TX_PIN);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  rakSerial.begin(UART_BAUDRATE);
  
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, HIGH);
  
  // Initialize ADC
  analogReadResolution(12); // Set ADC resolution to 12-bit
  
  Serial.println("==========================================================");
  Serial.println("WCS6800 Current Sensor LoRaWAN Monitor");
  Serial.println("==========================================================");
  Serial.print("UART: TX=GP"); Serial.print(UART_TX_PIN);
  Serial.print(", RX=GP"); Serial.print(UART_RX_PIN);
  Serial.print(" @ "); Serial.print(UART_BAUDRATE); Serial.println(" baud");
  Serial.print("ADC: WCS6800 on GP"); Serial.println(ADC_PIN);
  Serial.print("Device Address: "); Serial.println(DevAdd);
  Serial.println("==========================================================");
  
  // Initialize LoRaWAN
  if (!initializeLoRaWAN()) {
    Serial.println("FATAL ERROR: LoRaWAN initialization failed");
    return;
  }
  
  Serial.println("Starting main monitoring loop...");
}

void loop() {
  // Read current from sensor
  float currentVal = readCurrent();
  Serial.print("\nCurrent Reading: ");
  Serial.print(currentVal, 3);
  Serial.println(" A");
  
  // Send data via LoRaWAN
  if (sendLoRaWANPayload(currentVal)) {
    // Listen for downlink commands
    listenForDownlink();
  } else {
    Serial.println("Skipping downlink listen due to send failure");
  }
  
  // Wait before next transmission
  Serial.println("Waiting 60 seconds before next transmission...");
  delay(60000);
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

void clearUartBuffer() {
  delay(100);
  while (rakSerial.available()) {
    rakSerial.read();
  }
}

bool sendATCommand(const char* command, const char* expectedResponse = "OK", int timeout = 5000) {
  clearUartBuffer();
  
  rakSerial.print(command);
  rakSerial.print("\r\n");
  Serial.print("Sending command: ");
  Serial.println(command);
  
  unsigned long startTime = millis();
  String response = "";
  
  while ((millis() - startTime) < timeout) {
    if (rakSerial.available()) {
      char c = rakSerial.read();
      response += c;
      Serial.print(c);
      
      // Check if we got the expected response
      if (response.indexOf(expectedResponse) != -1) {
        return true;
      }
    }
    delay(10);
  }
  
  Serial.print("Timeout waiting for '");
  Serial.print(expectedResponse);
  Serial.println("' response");
  return false;
}

bool waitForModuleReady() {
  Serial.println("Waiting for RAK3172 module to be ready...");
  
  // Clear any boot messages
  delay(3000);
  clearUartBuffer();
  
  // Try to get a response to basic AT command
  int maxAttempts = 5;
  for (int attempt = 0; attempt < maxAttempts; attempt++) {
    Serial.print("Attempt ");
    Serial.print(attempt + 1);
    Serial.println(" to communicate with module...");
    
    if (sendATCommand("AT", "OK", 3000)) {
      Serial.println("Module is ready!");
      return true;
    }
    
    delay(1000);
  }
  
  Serial.println("Failed to communicate with module after multiple attempts");
  return false;
}

bool initializeLoRaWAN() {
  Serial.println("Initializing LoRaWAN ABP mode...");
  
  // Wait for module to be ready
  if (!waitForModuleReady()) {
    Serial.println("ERROR: Module not responding to AT commands");
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
    Serial.print("Setting ");
    Serial.print(commands[i].description);
    Serial.println("...");
    if (!sendATCommand(commands[i].command, "OK", 3000)) {
      Serial.print("ERROR: Failed to set ");
      Serial.println(commands[i].description);
      return false;
    }
    delay(500);
  }
  
  // Set device address
  Serial.println("Setting device address...");
  if (!sendATCommand(devAddrCmd.c_str(), "OK", 3000)) {
    Serial.println("ERROR: Failed to set device address");
    return false;
  }
  delay(500);
  
  // Set app session key
  Serial.println("Setting app session key...");
  if (!sendATCommand(appSKeyCmd.c_str(), "OK", 3000)) {
    Serial.println("ERROR: Failed to set app session key");
    return false;
  }
  delay(500);
  
  // Set network session key
  Serial.println("Setting network session key...");
  if (!sendATCommand(nwkSKeyCmd.c_str(), "OK", 3000)) {
    Serial.println("ERROR: Failed to set network session key");
    return false;
  }
  delay(500);
  
  // Join the network
  Serial.println("Joining LoRaWAN network...");
  if (sendATCommand("AT+JOIN", "OK", 10000)) {
    Serial.println("LoRaWAN initialized and joined successfully!");
    return true;
  } else {
    Serial.println("ERROR: Failed to join LoRaWAN network");
    return false;
  }
}

bool sendLoRaWANPayload(float currentValue) {
  // Convert float current_value to a compact binary payload.
  // Use a 2-byte signed integer for current in milliAmps (mA).
  int16_t currentMA = int16_t(currentValue * 1000); // Convert Amps to milliAmps
  
  // Ensure value fits within int16_t range
  if (currentMA > 32767) currentMA = 32767;
  if (currentMA < -32768) currentMA = -32768;
  
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
  Serial.print("Sending payload: ");
  Serial.print(hexPayload);
  Serial.print(" (Current: ");
  Serial.print(currentValue, 3);
  Serial.println("A)");
  
  if (sendATCommand(command.c_str(), "OK", 10000)) {
    Serial.println("Payload sent successfully");
    return true;
  } else {
    Serial.println("ERROR: Failed to send payload");
    return false;
  }
}

bool listenForDownlink() {
  Serial.println("Listening for downlink messages...");
  int timeoutCount = 0;
  int maxTimeout = 15000; // Wait up to 15 seconds for downlink
  
  while (timeoutCount < maxTimeout) {
    if (rakSerial.available()) {
      String downlinkData = rakSerial.readString();
      downlinkData.trim();
      Serial.print("Received: ");
      Serial.println(downlinkData);
      
      // Check for transmission done event
      if (downlinkData.indexOf("+EVT:TX_DONE") != -1) {
        Serial.println("Uplink transmission confirmed");
      }
      
      // Check for downlink event
      else if (downlinkData.indexOf("+EVT:RX_C") != -1 || downlinkData.indexOf("+EVT:RX_") != -1) {
        // Extract payload (last part after colons)
        int lastColonIndex = downlinkData.lastIndexOf(':');
        if (lastColonIndex != -1) {
          String payload = downlinkData.substring(lastColonIndex + 1);
          payload.trim();
          Serial.print("Downlink payload: ");
          Serial.println(payload);
          
          // Process LED commands
          if (payload == "01") {
            Serial.println("LED ON command received");
            digitalWrite(LED_PIN, HIGH);
          } else if (payload == "02") {
            Serial.println("LED OFF command received");
            digitalWrite(LED_PIN, LOW);
          } else if (payload == "04") {
            Serial.println("LED BLINK command received");
            for (int i = 0; i < 3; i++) { // Blink 3 times
              digitalWrite(LED_PIN, HIGH);
              delay(300);
              digitalWrite(LED_PIN, LOW);
              delay(300);
            }
          } else {
            Serial.print("Unknown command: ");
            Serial.println(payload);
          }
        }
        return true;
      }
    }
    
    delay(100);
    timeoutCount += 100;
  }
  
  Serial.println("No downlink received within timeout period");
  return false;
} 