#include "wifi_manager.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>

// Global variables - Using standard client for testing
WiFiClient wifiClient;  // Standard WiFi client for testing
PubSubClient mqttClient(wifiClient);
unsigned long lastWifiAttempt = 0;
unsigned long lastMqttAttempt = 0;
unsigned long lastMqttPublish = 0;
bool wifiConnected = false;
bool mqttConnected = false;
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

        // Setup MQTT if enabled
        if (MQTT_ENABLED) {
            if (setupMQTT()) {
                log_info("MQTT setup complete");
            } else {
                log_warning("MQTT setup failed");
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

    // Handle MQTT events if enabled
    if (MQTT_ENABLED && wifiConnected) {
        handleMQTTEvents();
    }
}

// MQTT callback function for incoming messages
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String topicStr = String(topic);
    String payloadStr = "";

    for (unsigned int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }

    log_format(LOG_INFO, "MQTT message received on topic: %s", topic);
    log_format(LOG_INFO, "MQTT payload: %s", payloadStr.c_str());

    processMQTTCommand(topicStr, payloadStr);
}

// Setup MQTT client
bool setupMQTT() {
    if (!MQTT_ENABLED || !wifiConnected) {
        return false;
    }

    log_info("Setting up MQTT...");

    // Configure MQTT client for testing (no TLS)
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);

    // Set buffer size for larger messages if needed
    mqttClient.setBufferSize(1024);

    // Set keep alive timeout
    mqttClient.setKeepAlive(60);

    log_format(LOG_INFO, "MQTT configured for testing broker: %s:%d", MQTT_SERVER, MQTT_PORT);
    return connectToMQTT();
}

// Connect to MQTT broker
bool connectToMQTT() {
    if (!MQTT_ENABLED || !wifiConnected) {
        log_error("MQTT connection prerequisites not met");
        return false;
    }

    // Prevent frequent reconnection attempts
    if (millis() - lastMqttAttempt < 5000) {
        return mqttConnected;
    }

    lastMqttAttempt = millis();

    log_format(LOG_INFO, "Connecting to MQTT broker: %s:%d", MQTT_SERVER, MQTT_PORT);

    // Test basic connectivity first
    log_format(LOG_INFO, "Testing connectivity to %s...", MQTT_SERVER);

    String clientId = String(MQTT_CLIENT_ID) + "_" + String(random(0xffff), HEX);
    log_format(LOG_INFO, "Using client ID: %s", clientId.c_str());

    bool connected = false;
    if (strlen(MQTT_USERNAME) > 0 && strlen(MQTT_PASSWORD) > 0) {
        log_info("Connecting with authentication...");
        connected = mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD);
    } else {
        log_info("Connecting without authentication...");
        connected = mqttClient.connect(clientId.c_str());
    }

    if (connected) {
        mqttConnected = true;
        log_format(LOG_INFO, "✅ MQTT connected successfully with client ID: %s", clientId.c_str());

        // Subscribe to command topic
        if (mqttClient.subscribe(MQTT_TOPIC_COMMAND)) {
            log_format(LOG_INFO, "Subscribed to command topic: %s", MQTT_TOPIC_COMMAND);
        } else {
            log_error("Failed to subscribe to command topic");
        }

        // Publish status message
        publishStatus("ONLINE");

        // Send a test message to verify connection
        publishTestMessage();

        return true;
    } else {
        mqttConnected = false;
        int state = mqttClient.state();
        log_format(LOG_ERROR, "❌ MQTT connection failed, state: %d", state);

        // Provide detailed error explanation
        switch(state) {
            case -4:
                log_error("   → Connection timeout");
                break;
            case -3:
                log_error("   → Connection lost");
                break;
            case -2:
                log_error("   → Connect failed (network issue)");
                log_error("   → Check: Internet connection, broker address, firewall");
                break;
            case -1:
                log_error("   → Disconnected");
                break;
            case 1:
                log_error("   → Bad protocol version");
                break;
            case 2:
                log_error("   → Client ID rejected");
                break;
            case 3:
                log_error("   → Server unavailable");
                break;
            case 4:
                log_error("   → Bad credentials");
                break;
            case 5:
                log_error("   → Unauthorized");
                break;
            default:
                log_format(LOG_ERROR, "   → Unknown error code: %d", state);
        }
        return false;
    }
}

// Handle MQTT events
void handleMQTTEvents() {
    if (!MQTT_ENABLED || !wifiConnected) {
        return;
    }

    if (!mqttClient.connected()) {
        mqttConnected = false;
        connectToMQTT();
    } else {
        mqttClient.loop();
    }
}

// Publish current data to MQTT
bool publishCurrentData(float currentValue) {
    if (!MQTT_ENABLED || !mqttConnected) {
        return false;
    }

    // Create JSON payload
    String jsonPayload = formatCurrentAsJson(currentValue);

    if (mqttClient.publish(MQTT_TOPIC_DATA, jsonPayload.c_str())) {
        log_format(LOG_DEBUG, "MQTT: Published current data: %s", jsonPayload.c_str());
        lastMqttPublish = millis();
        return true;
    } else {
        log_error("MQTT: Failed to publish current data");
        return false;
    }
}

// Publish status message to MQTT
bool publishStatus(const char* status) {
    if (!MQTT_ENABLED || !mqttConnected) {
        return false;
    }

    if (mqttClient.publish(MQTT_TOPIC_STATUS, status)) {
        log_format(LOG_DEBUG, "MQTT: Published status: %s", status);
        return true;
    } else {
        log_error("MQTT: Failed to publish status");
        return false;
    }
}

// Process MQTT command
void processMQTTCommand(String topic, String payload) {
    payload.trim();
    payload.toUpperCase();

    log_format(LOG_INFO, "Processing MQTT command: %s", payload.c_str());

    if (payload == "LED_ON" || payload == "01") {
        log_info("MQTT: LED ON command received");
        digitalWrite(LED_PIN, HIGH);
        publishStatus("LED_ON");
    }
    else if (payload == "LED_OFF" || payload == "02") {
        log_info("MQTT: LED OFF command received");
        digitalWrite(LED_PIN, LOW);
        publishStatus("LED_OFF");
    }
    else if (payload == "OTA_UPDATE" || payload == "03") {
        log_info("MQTT: OTA UPDATE command received");
        if (OTA_ENABLED) {
            publishStatus("OTA_TRIGGERED");
            // Trigger OTA update here
        } else {
            publishStatus("OTA_DISABLED");
        }
    }
    else if (payload == "LED_BLINK" || payload == "04") {
        log_info("MQTT: LED BLINK command received");
        // Blink LED 3 times
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED_PIN, HIGH);
            delay(200);
            digitalWrite(LED_PIN, LOW);
            delay(200);
        }
        publishStatus("LED_BLINK_COMPLETE");
    }
    else if (payload == "STATUS" || payload == "05") {
        log_info("MQTT: STATUS command received");
        publishStatus("SYSTEM_OK");
    }
    else if (payload == "TEST" || payload == "06") {
        log_info("MQTT: TEST command received");
        publishTestMessage();
        publishStatus("TEST_MESSAGE_SENT");
    }
    else if (payload == "DEBUG" || payload == "07") {
        log_info("MQTT: DEBUG command received");
        String debugInfo = "WiFi: " + getWiFiStatusString() + ", MQTT: Connected, Uptime: " + String(millis()/1000) + "s";
        publishDebugInfo(debugInfo.c_str());
        publishStatus("DEBUG_INFO_SENT");
    }
    else {
        log_format(LOG_WARNING, "MQTT: Unknown command: %s", payload.c_str());
        publishStatus("UNKNOWN_COMMAND");
    }
}

// Check if MQTT is connected
bool isMQTTConnected() {
    return mqttConnected;
}

// Publish test message to verify HiveMQ Cloud connection
bool publishTestMessage() {
    if (!MQTT_ENABLED || !mqttConnected) {
        return false;
    }

    // Create test JSON payload
    String testPayload = "{\"device\":\"WCS6800_Sensor\",\"message\":\"HiveMQ Cloud connection test\",\"timestamp\":" + String(millis()) + "}";

    if (mqttClient.publish(MQTT_TOPIC_TEST, testPayload.c_str())) {
        log_format(LOG_INFO, "MQTT: Published test message to HiveMQ Cloud: %s", testPayload.c_str());
        return true;
    } else {
        log_error("MQTT: Failed to publish test message to HiveMQ Cloud");
        return false;
    }
}

// Publish debug information
bool publishDebugInfo(const char* debugInfo) {
    if (!MQTT_ENABLED || !mqttConnected) {
        return false;
    }

    String debugPayload = "{\"device\":\"WCS6800_Sensor\",\"debug\":\"" + String(debugInfo) + "\",\"timestamp\":" + String(millis()) + "}";

    if (mqttClient.publish(MQTT_TOPIC_DEBUG, debugPayload.c_str())) {
        log_format(LOG_DEBUG, "MQTT: Published debug info: %s", debugInfo);
        return true;
    } else {
        log_error("MQTT: Failed to publish debug info");
        return false;
    }
}