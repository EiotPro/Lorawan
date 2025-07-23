#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- Device Information ---
#define DEVICE_NAME "WCS6800_Sensor"
#define DEVICE_VERSION "1.2.0"  // NEW VERSION - SIMULATING SUCCESSFUL OTA
#define BUILD_TIMESTAMP __DATE__ " " __TIME__

// --- Debug Configuration ---
#define DEBUG_LEVEL 3  // 0=OFF, 1=ERROR, 2=WARNING, 3=INFO, 4=DEBUG, 5=VERBOSE

// --- WCS6800 Sensor Configuration ---
#define ADC_PIN 26                // GPIO26 / ADC0
#define ADC_MAX_VALUE 4095.0
#define ADC_REF_VOLTAGE 3.3       // Volts
#define WCS6800_SENSITIVITY 0.0429 // V/A
#define WCS6800_OFFSET_VOLTAGE 1.65 // Volts (Output at 0A current)

// --- RAK3172 UART Communication Configuration ---
#define UART_TX_PIN 0             // GP0 (connected to RAK3172 RX)
#define UART_RX_PIN 1             // GP1 (connected to RAK3172 TX)
#define UART_BAUDRATE 115200

// --- LoRaWAN ABP Configuration ---
// Replace with your actual ABP credentials from ChirpStack
#define LORAWAN_DEV_ADDR "01d3257c"
#define LORAWAN_NWKS_KEY "06ebd62a3b4e2ed8d45d38d0f515988e"
#define LORAWAN_APPS_KEY "ef54ccd9b3d974e8736c60d916ad6e96"
#define LORAWAN_REGION 3          // IN865 region



// --- Pin Configuration ---
#define LED_PIN 25
#define RST_PIN 2

// --- Timing Configuration ---
#define LORA_TX_INTERVAL 60000    // LoRaWAN transmission interval in ms (60 seconds)
#define TX_INTERVAL 60000         // Legacy - same as LORA_TX_INTERVAL

// --- WiFi Configuration ---
#define WIFI_ENABLED true        // Set to true to enable WiFi
const char* const WIFI_SSID = "AEGEUS_2.4";
const char* const WIFI_PASSWORD = "aegeus2025";
#define WIFI_TX_INTERVAL 10000    // WiFi data transmission interval in ms



// --- OTA Configuration ---
#define OTA_ENABLED true
const char* const OTA_SERVER_URL = "http://iotlogic.in/ota/firmware.bin";
const char* const OTA_HTTP_USERNAME = "";
const char* const OTA_HTTP_PASSWORD = "";

#endif // CONFIG_H 