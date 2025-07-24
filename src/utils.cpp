#include "utils.h"
#include <Arduino.h>
#include <stdarg.h>
#include <LittleFS.h>
#include <WiFi.h>

// --- Logging Functions ---

void log_error(const char* msg) {
  log_message(LOG_ERROR, msg);
}

void log_warning(const char* msg) {
  log_message(LOG_WARNING, msg);
}

void log_info(const char* msg) {
  log_message(LOG_INFO, msg);
}

void log_debug(const char* msg) {
  log_message(LOG_DEBUG, msg);
}

void log_verbose(const char* msg) {
  log_message(LOG_VERBOSE, msg);
}

void log_message(uint8_t level, const char* msg) {
  if (level <= DEBUG_LEVEL) {
    const char* prefix = "";
    switch (level) {
      case LOG_ERROR:   prefix = "[ERROR] "; break;
      case LOG_WARNING: prefix = "[WARNING] "; break;
      case LOG_INFO:    prefix = "[INFO] "; break;
      case LOG_DEBUG:   prefix = "[DEBUG] "; break;
      case LOG_VERBOSE: prefix = "[VERBOSE] "; break;
    }

    Serial.print(prefix);
    Serial.println(msg);
  }
}

void log_format(uint8_t level, const char* format, ...) {
  if (level <= DEBUG_LEVEL) {
    char buffer[128]; // Buffer for the formatted string
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    log_message(level, buffer);
  }
}

// --- UART Helper Functions ---

void clearUartBuffer() {
  log_debug("Clearing UART buffer");
  delay(100);
  while (Serial1.available()) {
    Serial1.read();
  }
}

bool sendATCommand(const char* command, const char* expectedResponse, int timeout) {
  clearUartBuffer();

  Serial1.print(command);
  Serial1.print("\r\n");
  log_format(LOG_DEBUG, "Sending command: %s", command);

  unsigned long startTime = millis();
  String response = "";

  while ((millis() - startTime) < (unsigned long)timeout) {
    if (Serial1.available()) {
      char c = Serial1.read();
      response += c;

      // Print UART responses at VERBOSE level
      if (DEBUG_LEVEL >= LOG_VERBOSE) {
        Serial.print(c);
      }

      // Check if we got the expected response (or if no specific response expected)
      if (strlen(expectedResponse) == 0 || response.indexOf(expectedResponse) != -1) {
        if (strlen(expectedResponse) > 0) {
          log_debug("Command succeeded");
        } else {
          log_format(LOG_DEBUG, "Command sent, response: %s", response.c_str());
        }
        return true;
      }
    }
    delay(10);
  }

  if (strlen(expectedResponse) > 0) {
    log_format(LOG_ERROR, "Timeout waiting for '%s' response. Got: %s", expectedResponse, response.c_str());
  } else {
    log_format(LOG_DEBUG, "Command completed, response: %s", response.c_str());
  }
  return false;
}

// --- Diagnostic Functions ---

bool checkWCS6800Sensor() {
  log_info("Checking WCS6800 sensor...");
  
  // Read raw ADC value
  int adcValue = analogRead(ADC_PIN);
  
  // Convert ADC value to voltage
  float voltage = (float(adcValue) / ADC_MAX_VALUE) * ADC_REF_VOLTAGE;
  
  // Basic check - voltage should be around the offset voltage when no current flows
  if (voltage < 0.5 || voltage > 3.0) {
    log_format(LOG_ERROR, "WCS6800 sensor check failed. Voltage: %.2f V", voltage);
    return false;
  }
  
  log_format(LOG_INFO, "WCS6800 sensor check passed. Voltage: %.2f V", voltage);
  return true;
}

bool checkRAK3172Module() {
  log_info("Checking RAK3172 module...");
  
  // Try to get a response to basic AT command
  if (sendATCommand("AT", "OK", 3000)) {
    log_info("RAK3172 module check passed");
    return true;
  } else {
    log_error("RAK3172 module check failed");
    return false;
  }
}

bool runFullDiagnostics() {
  log_info("Running full diagnostics...");
  
  bool sensorOk = checkWCS6800Sensor();
  bool moduleOk = checkRAK3172Module();
  
  if (sensorOk && moduleOk) {
    log_info("All diagnostics passed");
    return true;
  } else {
    log_error("Diagnostics failed");
    return false;
  }
}

// --- Error Handling ---

void handleError(uint8_t errorCode) {
  switch (errorCode) {
    case ERR_SENSOR_READ:
      log_error("Sensor read error: Check WCS6800 connections");
      break;
    case ERR_LORAWAN_INIT:
      log_error("LoRaWAN initialization error: Check RAK3172 connections");
      break;
    case ERR_LORAWAN_JOIN:
      log_error("LoRaWAN join error: Check credentials and coverage");
      break;
    case ERR_LORAWAN_SEND:
      log_error("LoRaWAN send error: Check connection and signal strength");
      break;
    case ERR_WIFI_CONNECT:
      log_error("WiFi connection error: Check SSID and password");
      break;
    case ERR_MQTT_CONNECT:
      log_error("MQTT connection error: Check broker settings");
      break;
    case ERR_BT_INIT:
      log_error("Bluetooth initialization error");
      break;
    case ERR_OTA_DOWNLOAD:
      log_error("OTA download error: Check server URL and connectivity");
      break;
    case ERR_OTA_VERIFY:
      log_error("OTA verification error: Firmware may be corrupted");
      break;
    default:
      log_error("Unknown error occurred");
      break;
  }
}

bool retryOperation(uint8_t errorCode, uint8_t maxRetries) {
  for (uint8_t retry = 0; retry < maxRetries; retry++) {
    log_format(LOG_INFO, "Retry attempt %d/%d", retry + 1, maxRetries);
    
    switch (errorCode) {
      case ERR_WIFI_CONNECT:
        // WiFi reconnect logic would go here
        log_info("Attempting WiFi reconnection...");
        delay(1000);
        break;
      case ERR_LORAWAN_JOIN:
        log_info("Attempting LoRaWAN rejoin...");
        delay(5000);
        break;
      // Add other retry logic for different error codes
      default:
        log_error("No retry logic for this error code");
        return false;
    }
  }
  
  log_error("Max retry attempts reached");
  return false;
}

// --- JSON Formatting ---

String formatCurrentAsJson(float currentValue) {
  String jsonString = "{";
  jsonString += "\"current\":" + String(currentValue, 3);
  jsonString += ",\"timestamp\":" + String(millis());
  jsonString += ",\"unit\":\"A\"";
  jsonString += "}";
  
  log_format(LOG_DEBUG, "JSON: %s", jsonString.c_str());
  return jsonString;
}

// --- System Monitoring Functions ---

void printSystemInfo() {
  log_info("=== SYSTEM INFORMATION ===");
  log_format(LOG_INFO, "Device: %s", DEVICE_NAME);
  log_format(LOG_INFO, "Version: %s", DEVICE_VERSION);
  log_format(LOG_INFO, "Build: %s", BUILD_TIMESTAMP);
  log_format(LOG_INFO, "Uptime: %.2f hours", getSystemUptime());
  log_format(LOG_INFO, "Free Heap: %d bytes", getFreeHeap());

  // Print pin configuration
  log_format(LOG_INFO, "ADC Pin: GP%d", ADC_PIN);
  log_format(LOG_INFO, "LED Pin: GP%d", LED_PIN);
  log_format(LOG_INFO, "UART TX: GP%d, RX: GP%d", UART_TX_PIN, UART_RX_PIN);

  // Print feature status
  log_format(LOG_INFO, "WiFi: %s", WIFI_ENABLED ? "Enabled" : "Disabled");
  log_format(LOG_INFO, "BLE: %s", BLE_ENABLED ? "Enabled" : "Disabled");
  log_format(LOG_INFO, "OTA: %s", OTA_ENABLED ? "Enabled" : "Disabled");
  log_format(LOG_INFO, "MQTT: %s", MQTT_ENABLED ? "Enabled" : "Disabled");
  log_info("========================");
}

void printMemoryInfo() {
  log_info("=== MEMORY INFORMATION ===");
  log_format(LOG_INFO, "Free Heap: %d bytes", getFreeHeap());
  log_format(LOG_INFO, "Stack Usage: Estimated");

  // Print buffer sizes
  log_format(LOG_INFO, "UART Buffer: Available");
  log_format(LOG_INFO, "Log Buffer: 128 bytes");
  log_info("=========================");
}

void printFileSystemInfo() {
  log_info("=== FILESYSTEM INFORMATION ===");

  if (!LittleFS.begin()) {
    log_error("LittleFS not available");
    return;
  }

  // Get filesystem info
  FSInfo fsInfo;
  LittleFS.info(fsInfo);

  log_format(LOG_INFO, "Total Size: %d bytes", fsInfo.totalBytes);
  log_format(LOG_INFO, "Used Size: %d bytes", fsInfo.usedBytes);
  log_format(LOG_INFO, "Free Size: %d bytes", fsInfo.totalBytes - fsInfo.usedBytes);
  log_format(LOG_INFO, "Block Size: %d bytes", fsInfo.blockSize);
  log_format(LOG_INFO, "Page Size: %d bytes", fsInfo.pageSize);

  // List files
  log_info("Files:");
  File root = LittleFS.open("/", "r");
  if (root && root.isDirectory()) {
    File file = root.openNextFile();
    while (file) {
      if (!file.isDirectory()) {
        log_format(LOG_INFO, "  %s (%d bytes)", file.name(), file.size());
      }
      file = root.openNextFile();
    }
  }

  log_info("==============================");
}

float getSystemUptime() {
  return millis() / 3600000.0; // Convert to hours
}

uint32_t getFreeHeap() {
  return rp2040.getFreeHeap();
}

float getCPUTemperature() {
  // Note: This is a placeholder - actual temperature reading would require
  // specific implementation for RP2040
  return 25.0; // Placeholder temperature
}

// --- Advanced Error Handling ---

void logSystemError(const char* component, const char* error) {
  log_format(LOG_ERROR, "[%s] %s", component, error);

  // Save error to file if filesystem is available
  saveErrorLog(error);

  // Create detailed error report
  createErrorReport();
}

void createErrorReport() {
  if (!LittleFS.begin()) {
    return;
  }

  File errorReport = LittleFS.open("/error_report.txt", "w");
  if (!errorReport) {
    return;
  }

  errorReport.printf("=== ERROR REPORT ===\n");
  errorReport.printf("Timestamp: %lu ms\n", millis());
  errorReport.printf("Device: %s\n", DEVICE_NAME);
  errorReport.printf("Version: %s\n", DEVICE_VERSION);
  errorReport.printf("Uptime: %.2f hours\n", getSystemUptime());
  errorReport.printf("Free Heap: %d bytes\n", getFreeHeap());
  errorReport.printf("WiFi Status: %s\n", getWiFiStatusString().c_str());
  errorReport.printf("MQTT Status: %s\n", getMQTTStatusString().c_str());
  errorReport.printf("BLE Status: %s\n", getBLEStatusString().c_str());
  errorReport.printf("==================\n");

  errorReport.close();
  log_info("Error report created: /error_report.txt");
}

bool saveErrorLog(const char* error) {
  if (!LittleFS.begin()) {
    return false;
  }

  File errorLog = LittleFS.open("/error.log", "a");
  if (!errorLog) {
    return false;
  }

  errorLog.printf("[%lu] %s\n", millis(), error);
  errorLog.close();

  return true;
}

void clearErrorLog() {
  if (!LittleFS.begin()) {
    return;
  }

  if (LittleFS.exists("/error.log")) {
    LittleFS.remove("/error.log");
    log_info("Error log cleared");
  }

  if (LittleFS.exists("/error_report.txt")) {
    LittleFS.remove("/error_report.txt");
    log_info("Error report cleared");
  }
}

// --- Data Processing Functions ---

float calculateMovingAverage(float newValue, float* buffer, size_t bufferSize, size_t* index) {
  // Add new value to circular buffer
  buffer[*index] = newValue;
  *index = (*index + 1) % bufferSize;

  // Calculate average
  float sum = 0.0;
  for (size_t i = 0; i < bufferSize; i++) {
    sum += buffer[i];
  }

  return sum / bufferSize;
}

bool validateCurrentReading(float current) {
  // Check for reasonable current values
  if (current < -50.0 || current > 50.0) {
    log_format(LOG_WARNING, "Current reading out of range: %.3f A", current);
    return false;
  }

  // Check for NaN or infinite values
  if (isnan(current) || isinf(current)) {
    log_error("Invalid current reading: NaN or Infinite");
    return false;
  }

  return true;
}

String formatSystemStatus() {
  String status = "{";
  status += "\"uptime\":" + String(getSystemUptime(), 2);
  status += ",\"free_heap\":" + String(getFreeHeap());
  status += ",\"wifi\":\"" + getWiFiStatusString() + "\"";
  status += ",\"mqtt\":\"" + getMQTTStatusString() + "\"";
  status += ",\"ble\":\"" + getBLEStatusString() + "\"";
  status += ",\"version\":\"" + String(DEVICE_VERSION) + "\"";
  status += "}";
  return status;
}

void performSystemHealthCheck() {
  log_info("=== SYSTEM HEALTH CHECK ===");

  // Check memory
  uint32_t freeHeap = getFreeHeap();
  if (freeHeap < 10000) { // Less than 10KB free
    log_format(LOG_WARNING, "Low memory warning: %d bytes free", freeHeap);
  } else {
    log_format(LOG_INFO, "Memory status: OK (%d bytes free)", freeHeap);
  }

  // Check sensors
  if (checkWCS6800Sensor()) {
    log_info("WCS6800 sensor: OK");
  } else {
    log_error("WCS6800 sensor: FAILED");
  }

  // Check LoRa module
  if (checkRAK3172Module()) {
    log_info("RAK3172 module: OK");
  } else {
    log_error("RAK3172 module: FAILED");
  }

  // Check filesystem
  if (LittleFS.begin()) {
    log_info("Filesystem: OK");
  } else {
    log_warning("Filesystem: Not available");
  }

  log_info("=========================");
}

// --- Network Utilities ---

String getWiFiStatusString() {
  if (!WIFI_ENABLED) {
    return "DISABLED";
  }

  switch (WiFi.status()) {
    case WL_CONNECTED:
      return "CONNECTED";
    case WL_DISCONNECTED:
      return "DISCONNECTED";
    case WL_CONNECT_FAILED:
      return "CONNECT_FAILED";
    case WL_CONNECTION_LOST:
      return "CONNECTION_LOST";
    case WL_NO_SSID_AVAIL:
      return "NO_SSID_AVAILABLE";
    default:
      return "UNKNOWN";
  }
}

String getMQTTStatusString() {
  if (!MQTT_ENABLED) {
    return "DISABLED";
  }

  // Note: This would require access to MQTT client state
  // For now, return a placeholder
  return "UNKNOWN";
}

String getBLEStatusString() {
  if (!BLE_ENABLED) {
    return "DISABLED";
  }

  // Note: This would require access to BLE state
  // For now, return a placeholder
  return "UNKNOWN";
}

// --- Integration Test Functions ---

void runIntegrationTests() {
  log_info("=== RUNNING INTEGRATION TESTS ===");

  // Test 1: System Information
  log_info("Test 1: System Information");
  printSystemInfo();
  delay(1000);

  // Test 2: Memory Information
  log_info("Test 2: Memory Information");
  printMemoryInfo();
  delay(1000);

  // Test 3: Filesystem Information
  log_info("Test 3: Filesystem Information");
  printFileSystemInfo();
  delay(1000);

  // Test 4: System Health Check
  log_info("Test 4: System Health Check");
  performSystemHealthCheck();
  delay(1000);

  // Test 5: Data Processing
  log_info("Test 5: Data Processing Functions");
  testDataProcessing();
  delay(1000);

  // Test 6: Error Handling
  log_info("Test 6: Error Handling");
  testErrorHandling();
  delay(1000);

  // Test 7: Network Status
  log_info("Test 7: Network Status");
  testNetworkStatus();
  delay(1000);

  log_info("=== INTEGRATION TESTS COMPLETED ===");
}

void testDataProcessing() {
  log_info("Testing data processing functions...");

  // Test moving average
  float buffer[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
  size_t index = 0;

  for (int i = 1; i <= 10; i++) {
    float value = i * 1.5;
    float avg = calculateMovingAverage(value, buffer, 5, &index);
    log_format(LOG_INFO, "Value: %.2f, Moving Average: %.2f", value, avg);
  }

  // Test current validation
  float testCurrents[] = {0.5, 25.0, -10.0, 100.0, NAN};
  for (int i = 0; i < 5; i++) {
    bool valid = validateCurrentReading(testCurrents[i]);
    log_format(LOG_INFO, "Current %.2f A: %s", testCurrents[i], valid ? "VALID" : "INVALID");
  }

  // Test JSON formatting
  String jsonData = formatCurrentAsJson(12.345);
  log_format(LOG_INFO, "JSON Data: %s", jsonData.c_str());

  String systemStatus = formatSystemStatus();
  log_format(LOG_INFO, "System Status: %s", systemStatus.c_str());
}

void testErrorHandling() {
  log_info("Testing error handling functions...");

  // Test error logging
  logSystemError("TEST_COMPONENT", "This is a test error message");

  // Test error log saving
  bool saved = saveErrorLog("Integration test error log entry");
  log_format(LOG_INFO, "Error log save: %s", saved ? "SUCCESS" : "FAILED");

  // Test error report creation
  createErrorReport();
  log_info("Error report created");

  // Note: We won't clear the error log in tests to preserve the test data
}

void testNetworkStatus() {
  log_info("Testing network status functions...");

  String wifiStatus = getWiFiStatusString();
  log_format(LOG_INFO, "WiFi Status: %s", wifiStatus.c_str());

  String mqttStatus = getMQTTStatusString();
  log_format(LOG_INFO, "MQTT Status: %s", mqttStatus.c_str());

  String bleStatus = getBLEStatusString();
  log_format(LOG_INFO, "BLE Status: %s", bleStatus.c_str());
}