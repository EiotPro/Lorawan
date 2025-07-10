#include "utils.h"
#include <Arduino.h>
#include <stdarg.h>

// --- Logging Functions ---

void log_error(const char* msg) {
  log_message(LOG_ERROR, msg);
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
      
      // Check if we got the expected response
      if (response.indexOf(expectedResponse) != -1) {
        log_debug("Command succeeded");
        return true;
      }
    }
    delay(10);
  }
  
  log_format(LOG_ERROR, "Timeout waiting for '%s' response", expectedResponse);
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