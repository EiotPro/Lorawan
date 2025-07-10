#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- Debug Configuration ---
#define DEBUG_LEVEL 2  // 0=OFF, 1=ERROR, 2=INFO, 3=DEBUG, 4=VERBOSE

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

// --- Transmission Configuration ---
#define TX_INTERVAL 60000         // Transmit interval in milliseconds (60 seconds)

// --- WiFi Configuration ---
#define WIFI_ENABLED false         // Set to true to enable WiFi
const char* const WIFI_SSID = "YourSSID";
const char* const WIFI_PASSWORD = "YourPassword";
#define WIFI_TX_INTERVAL 10000    // WiFi data transmission interval in ms

// --- MQTT Configuration ---
#define MQTT_ENABLED false         // Set to true to enable MQTT
const char* const MQTT_BROKER = "192.168.1.100";
#define MQTT_PORT 1883
const char* const MQTT_USERNAME = "user";
const char* const MQTT_PASSWORD = "password";
const char* const MQTT_TOPIC = "wcs6800/data";

// --- Bluetooth Configuration ---
#define BLE_ENABLED false          // Set to true to enable BLE when supported
const char* const BLE_DEVICE_NAME = "WCS6800_Monitor";
#define BLE_TX_INTERVAL 5000      // Bluetooth data transmission interval in ms

// --- OTA Configuration ---
#define OTA_ENABLED false          // Set to true to enable OTA when supported
const char* const OTA_SERVER_URL = "http://192.168.1.100:8080/firmware.bin";
const char* const OTA_HTTP_USERNAME = "admin";
const char* const OTA_HTTP_PASSWORD = "admin";

#endif // CONFIG_H 