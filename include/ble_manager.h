#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "utils.h"

// BLE is currently not fully supported on Pico W with Earlephilhower core
// These functions provide stubs for compatibility

// Function prototypes
bool setupBLE();
bool updateBLECurrentValue(float currentValue);
void handleBLEEvents();
void processBLECommand(String command);

#endif // BLE_MANAGER_H 