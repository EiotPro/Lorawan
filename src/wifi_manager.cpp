#include "wifi_manager.h"
#include <ArduinoJson.h>

// Global variables
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
unsigned long lastWifiAttempt = 0;
bool wifiConnected = false;
uint8_t wifiState = WIFI_STATE_DISCONNECTED;

// Set up WiFi and MQTT
bool setupWiFi() {
    if (!WIFI_ENABLED) {
        log_info("WiFi disabled in configuration");
        return false;
    }

    log_info("Setting up WiFi...");
    // Earlephilhower WiFi doesn't have mode() function, it defaults to station mode
    
    if (connectToWiFi()) {
        log_info("WiFi setup complete");
        
        // Setup MQTT if enabled
        if (MQTT_ENABLED) {
            if (setupMQTT()) {
                log_info("MQTT setup complete");
            } else {
                log_error("MQTT setup failed");
            }
        }
        
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

// Set up MQTT client
bool setupMQTT() {
    if (!MQTT_ENABLED) {
        return false;
    }
    
    log_info("Setting up MQTT client...");
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(onMQTTMessage);
    
    return true;
}

// Publish current data to MQTT broker
bool publishCurrentData(float currentValue) {
    if (!WIFI_ENABLED || !MQTT_ENABLED) {
        return false;
    }
    
    // Check if connected to WiFi and MQTT
    if (!wifiConnected) {
        if (!connectToWiFi()) {
            return false;
        }
    }
    
    // Connect to MQTT if not connected
    if (!mqttClient.connected()) {
        log_info("Connecting to MQTT broker...");
        
        // Create a client ID based on timestamp
        String clientId = "WCS6800_";
        clientId += String(millis());
        
        if (mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
            log_info("MQTT connected");
        } else {
            log_format(LOG_ERROR, "MQTT connection failed, rc=%d", mqttClient.state());
            handleError(ERR_MQTT_CONNECT);
            return false;
        }
    }
    
    // Format data as JSON
    String jsonData = formatCurrentAsJson(currentValue);
    
    // Publish to MQTT topic
    log_format(LOG_INFO, "Publishing to MQTT topic: %s", MQTT_TOPIC);
    if (mqttClient.publish(MQTT_TOPIC, jsonData.c_str())) {
        log_info("MQTT publish successful");
        return true;
    } else {
        log_error("MQTT publish failed");
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
    
    // Process MQTT messages if connected
    if (wifiConnected && MQTT_ENABLED && mqttClient.connected()) {
        mqttClient.loop();
    }
}

// MQTT message callback
void onMQTTMessage(char* topic, byte* payload, unsigned int length) {
    // Create a null-terminated string from the payload
    // Using fixed buffer to avoid VLA warning
    char message[256]; // Using fixed size buffer instead of VLA
    unsigned int copyLength = (length < 255) ? length : 255; // Prevent buffer overflow
    memcpy(message, payload, copyLength);
    message[copyLength] = '\0';
    
    log_format(LOG_INFO, "MQTT message received on topic: %s", topic);
    log_format(LOG_INFO, "Message: %s", message);
    
    // Process MQTT messages as needed
    // For now, just log them, but could add additional logic here
} 