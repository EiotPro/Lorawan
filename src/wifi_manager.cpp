#include "wifi_manager.h"
#include <ArduinoJson.h>

// Global variables
WiFiClient wifiClient;
unsigned long lastWifiAttempt = 0;
bool wifiConnected = false;
uint8_t wifiState = WIFI_STATE_DISCONNECTED;

// Set up WiFi
bool setupWiFi() {
    if (!WIFI_ENABLED) {
        log_info("WiFi disabled in configuration");
        return false;
    }

    log_info("Setting up WiFi...");
    // Earlephilhower WiFi doesn't have mode() function, it defaults to station mode
    
    if (connectToWiFi()) {
        log_info("WiFi setup complete");
        

        
        return true;
    } else {
        log_error("WiFi setup failed");
        return false;
    }
}

// Connect to WiFi network
bool connectToWiFi() {
    // Prevent frequent reconnection attempts
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        wifiState = WIFI_STATE_CONNECTED;
        return true;
    }

    if (millis() - lastWifiAttempt < 10000) {
        return false;  // Don't try to connect more than once every 10 seconds
    }
    
    lastWifiAttempt = millis();
    wifiState = WIFI_STATE_CONNECTING;
    
    log_format(LOG_INFO, "Connecting to WiFi network: %s", WIFI_SSID);
    
    // Use Earlephilhower WiFi implementation
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Wait for connection with timeout
    uint8_t attempt = 0;
    const uint8_t maxAttempts = 20;  // 10 seconds timeout (20 * 500ms)
    
    while (WiFi.status() != WL_CONNECTED && attempt < maxAttempts) {
        delay(500);
        log_verbose(".");
        attempt++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        wifiState = WIFI_STATE_CONNECTED;
        log_format(LOG_INFO, "WiFi connected with IP: %s", WiFi.localIP().toString().c_str());
        return true;
    } else {
        wifiConnected = false;
        wifiState = WIFI_STATE_DISCONNECTED;
        log_error("Failed to connect to WiFi");
        handleError(ERR_WIFI_CONNECT);
        return false;
    }
}



// Handle WiFi events
void handleWiFiEvents() {
    if (!WIFI_ENABLED) {
        return;
    }
    
    // Check WiFi connection and reconnect if needed
    if (WiFi.status() != WL_CONNECTED) {
        wifiConnected = false;
        wifiState = WIFI_STATE_DISCONNECTED;
        log_debug("WiFi disconnected, attempting to reconnect...");
        connectToWiFi();
    }
    
}