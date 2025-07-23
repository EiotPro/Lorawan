#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "utils.h"

// WiFi connection states
#define WIFI_STATE_DISCONNECTED 0
#define WIFI_STATE_CONNECTED 1
#define WIFI_STATE_CONNECTING 2

// Function prototypes
bool setupWiFi();
bool connectToWiFi();
void handleWiFiEvents();

#endif // WIFI_MANAGER_H 