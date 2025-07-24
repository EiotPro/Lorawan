#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include "config.h"

// --- Debug Levels ---
#define LOG_NONE 0
#define LOG_ERROR 1
#define LOG_WARNING 2
#define LOG_INFO 3
#define LOG_DEBUG 4
#define LOG_VERBOSE 5

// --- Error Codes ---
#define ERR_SENSOR_READ 1
#define ERR_LORAWAN_INIT 2
#define ERR_LORAWAN_JOIN 3
#define ERR_LORAWAN_SEND 4
#define ERR_WIFI_CONNECT 5
#define ERR_MQTT_CONNECT 6
#define ERR_BT_INIT 7
#define ERR_OTA_DOWNLOAD 8
#define ERR_OTA_VERIFY 9

// Logging functions
void log_error(const char* msg);
void log_warning(const char* msg);
void log_info(const char* msg);
void log_debug(const char* msg);
void log_verbose(const char* msg);
void log_message(uint8_t level, const char* msg);
void log_format(uint8_t level, const char* format, ...);

// UART helper functions
void clearUartBuffer();
bool sendATCommand(const char* command, const char* expectedResponse, int timeout);

// Diagnostic functions
bool checkWCS6800Sensor();
bool checkRAK3172Module();
bool runFullDiagnostics();

// Error handling
void handleError(uint8_t errorCode);
bool retryOperation(uint8_t errorCode, uint8_t maxRetries);

// Data formatting
String formatCurrentAsJson(float currentValue);

// System monitoring functions
void printSystemInfo();
void printMemoryInfo();
void printFileSystemInfo();
float getSystemUptime();
uint32_t getFreeHeap();
float getCPUTemperature();

// Advanced error handling
void logSystemError(const char* component, const char* error);
void createErrorReport();
bool saveErrorLog(const char* error);
void clearErrorLog();

// Data processing functions
float calculateMovingAverage(float newValue, float* buffer, size_t bufferSize, size_t* index);
bool validateCurrentReading(float current);
String formatSystemStatus();
void performSystemHealthCheck();

// Network utilities
String getWiFiStatusString();
String getMQTTStatusString();
String getBLEStatusString();

// Integration test functions
void runIntegrationTests();
void testDataProcessing();
void testErrorHandling();
void testNetworkStatus();

#endif // UTILS_H