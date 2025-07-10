#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "utils.h"

// OTA Status codes
#define OTA_STATUS_IDLE 0
#define OTA_STATUS_WIFI 1
#define OTA_STATUS_DOWNLOAD 2
#define OTA_STATUS_VERIFY 3
#define OTA_STATUS_APPLYING 4
#define OTA_STATUS_SUCCESS 5
#define OTA_STATUS_FAILED 6

// Function prototypes
bool setupOTA();
bool triggerOTAUpdate(uint8_t method);
bool downloadAndApplyOTA();
bool verifyFirmware(uint8_t* buffer, size_t size);
void handleOTAEvents();
uint8_t getOTAStatus();

// OTA Update Methods
#define OTA_METHOD_WIFI 1
#define OTA_METHOD_BLE 2

#endif // OTA_MANAGER_H 