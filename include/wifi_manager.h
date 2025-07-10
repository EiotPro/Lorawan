#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include "utils.h"

// WiFi connection states
#define WIFI_STATE_DISCONNECTED 0
#define WIFI_STATE_CONNECTED 1
#define WIFI_STATE_CONNECTING 2

// Function prototypes
bool setupWiFi();
bool connectToWiFi();
bool setupMQTT();
bool publishCurrentData(float currentValue);
void handleWiFiEvents();
void onMQTTMessage(char* topic, byte* payload, unsigned int length);

#endif // WIFI_MANAGER_H 