# LoRaWAN-Android App Synchronization Requirements

## Overview

This document defines the comprehensive requirements for synchronizing the Raspberry Pi Pico W LoRaWAN device with the AmpMeter Android application. It serves as a complete reference for implementing bidirectional communication, configuration management, and data exchange between both systems.

## Configuration Parameters Mapping

The following configuration parameters must be synchronized between the Android app and the LoRaWAN device:

### Device Identification
| Android Parameter | LoRaWAN Configuration | Description |
|-------------------|------------------------|-------------|
| `deviceId` | `DEVICE_ID` | Unique identifier for the device |
| `deviceName` | `BLE_DEVICE_NAME` | Human-readable device name |

### LoRaWAN Settings
| Android Parameter | LoRaWAN Configuration | Description |
|-------------------|------------------------|-------------|
| `lorawanDevAddr` | `LORAWAN_DEV_ADDR` | Device address (ABP mode) |
| `lorawanNwksKey` | `LORAWAN_NWKS_KEY` | Network session key |
| `lorawanAppsKey` | `LORAWAN_APPS_KEY` | Application session key |
| `lorawanRegion` | `LORAWAN_REGION` | Region code (0=EU868, 1=US915, 2=AU915, 3=IN865, 4=AS923) |
| `txInterval` | `TX_INTERVAL` | Transmission interval in milliseconds |

### WiFi Settings
| Android Parameter | LoRaWAN Configuration | Description |
|-------------------|------------------------|-------------|
| `wifiEnabled` | `WIFI_ENABLED` | Enable/disable WiFi connectivity |
| `wifiSsid` | `WIFI_SSID` | WiFi network name |
| `wifiPassword` | `WIFI_PASSWORD` | WiFi network password |
| `wifiTxInterval` | `WIFI_TX_INTERVAL` | Data transmission interval over WiFi (ms) |

### MQTT Settings
| Android Parameter | LoRaWAN Configuration | Description |
|-------------------|------------------------|-------------|
| `mqttEnabled` | `MQTT_ENABLED` | Enable/disable MQTT communication |
| `mqttBrokerUrl` | `MQTT_BROKER` | MQTT broker address |
| `mqttPort` | `MQTT_PORT` | MQTT broker port (default: 1883) |
| `mqttUsername` | `MQTT_USERNAME` | MQTT authentication username |
| `mqttPassword` | `MQTT_PASSWORD` | MQTT authentication password |
| `mqttTopic` | `MQTT_TOPIC` | MQTT topic for publishing/subscribing |

### Bluetooth Settings
| Android Parameter | LoRaWAN Configuration | Description |
|-------------------|------------------------|-------------|
| `bleEnabled` | `BLE_ENABLED` | Enable/disable BLE functionality |
| `bleDeviceName` | `BLE_DEVICE_NAME` | BLE device name for advertising |
| `bleTxInterval` | `BLE_TX_INTERVAL` | BLE data transmission interval (ms) |

### OTA Update Settings
| Android Parameter | LoRaWAN Configuration | Description |
|-------------------|------------------------|-------------|
| `otaEnabled` | `OTA_ENABLED` | Enable/disable OTA updates |
| `otaServerUrl` | `OTA_SERVER_URL` | URL for firmware updates |
| `otaHttpUsername` | `OTA_HTTP_USERNAME` | OTA server authentication username |
| `otaHttpPassword` | `OTA_HTTP_PASSWORD` | OTA server authentication password |

### Sensor Calibration
| Android Parameter | LoRaWAN Configuration | Description |
|-------------------|------------------------|-------------|
| `wcs6800Sensitivity` | `WCS6800_SENSITIVITY` | Current sensor sensitivity in V/A (default: 0.0429) |
| `wcs6800OffsetVoltage` | `WCS6800_OFFSET_VOLTAGE` | Zero current reference voltage in V (default: 1.65) |
| `adcRefVoltage` | `ADC_REF_VOLTAGE` | ADC reference voltage in V (default: 3.3) |

### Advanced Settings
| Android Parameter | LoRaWAN Configuration | Description |
|-------------------|------------------------|-------------|
| `debugLevel` | `DEBUG_LEVEL` | Debug verbosity (0=OFF, 1=ERROR, 2=INFO, 3=DEBUG, 4=VERBOSE) |
| `useAdaptiveSamplingRate` | `USE_ADAPTIVE_RATE` | Enable/disable adaptive sampling based on activity |
| `measurementInterval` | `MEASUREMENT_INTERVAL` | Time between sensor readings (ms) |
| `dataFormat` | `DATA_FORMAT` | Data format ("JSON" or "COMPACT") |

## Communication Protocol Implementation

### BLE Communication

The LoRaWAN device must implement a BLE service with the following characteristics to enable configuration:

```cpp
// UUIDs for the BLE services and characteristics
#define SERVICE_UUID                  "2456e1b9-26e2-8f83-e744-f34f01e9d701"
#define DEVICE_CONFIG_UUID            "2456e1b9-26e2-8f83-e744-f34f01e9d702"
#define LORAWAN_CONFIG_UUID           "2456e1b9-26e2-8f83-e744-f34f01e9d703"
#define WIFI_CONFIG_UUID              "2456e1b9-26e2-8f83-e744-f34f01e9d704"
#define MQTT_CONFIG_UUID              "2456e1b9-26e2-8f83-e744-f34f01e9d705"
#define BLE_CONFIG_UUID               "2456e1b9-26e2-8f83-e744-f34f01e9d706"
#define OTA_CONFIG_UUID               "2456e1b9-26e2-8f83-e744-f34f01e9d707"
#define SENSOR_CONFIG_UUID            "2456e1b9-26e2-8f83-e744-f34f01e9d708"
#define ADVANCED_CONFIG_UUID          "2456e1b9-26e2-8f83-e744-f34f01e9d709"
#define SENSOR_DATA_UUID              "2456e1b9-26e2-8f83-e744-f34f01e9d70a"
```

Each characteristic must handle both read and write operations using JSON-formatted data:

```json
// Example JSON for LORAWAN_CONFIG_UUID
{
  "devAddr": "01d3257c",
  "nwksKey": "06ebd62a3b4e2ed8d45d38d0f515988e",
  "appsKey": "ef54ccd9b3d974e8736c60d916ad6e96",
  "region": 3,
  "txInterval": 60000
}
```

### LoRaWAN Communication

#### Uplink Payload Format

Binary encoding for LoRaWAN:
- Byte 0-1: Current value in mA (signed 16-bit, big-endian)
- Byte 2-3: Voltage value in cV (unsigned 16-bit, big-endian)
- Byte 4: Battery level percentage (0-100)
- Byte 5: RSSI as positive value (optional)
- Byte 6: SNR in 0.1dB units (optional)

#### Downlink Commands

The device must support downlink commands on port 2:
- Command 0x01: LED ON
- Command 0x02: LED OFF
- Command 0x03: Trigger OTA update
- Command 0x04: LED blink (diagnostic)
- Command 0x10-0xFF: Configuration commands

### Data Formats

The device must support two data formats for WiFi/MQTT/BLE transmission:

1. **JSON Format**:
```json
{
  "deviceId": "amp_meter_001",
  "current": 0.5342,
  "voltage": 3.3,
  "batteryLevel": 85,
  "timestamp": 1680000000
}
```

2. **Compact Format**:
```
amp_meter_001,0.5342,3.3,85,1680000000
```

## Configuration Storage Implementation

The device must implement persistent configuration storage using LittleFS with the following structure:

```json
{
  "device": {
    "id": "amp_meter_001",
    "name": "Current Sensor"
  },
  "lorawan": {
    "devAddr": "01d3257c",
    "nwksKey": "06ebd62a3b4e2ed8d45d38d0f515988e",
    "appsKey": "ef54ccd9b3d974e8736c60d916ad6e96",
    "region": 3,
    "txInterval": 60000
  },
  "wifi": {
    "enabled": true,
    "ssid": "AEGUES_2.4",
    "password": "aegeus2025",
    "txInterval": 10000
  },
  "mqtt": {
    "enabled": false,
    "broker": "103.161.75.85",
    "port": 1883,
    "username": "admin",
    "password": "admin",
    "topic": "ampmeter/data"
  },
  "ble": {
    "enabled": true,
    "deviceName": "AmpMeter_Device",
    "txInterval": 5000
  },
  "ota": {
    "enabled": false,
    "serverUrl": "http://103.161.75.85:8082/firmware.bin",
    "username": "admin",
    "password": "admin"
  },
  "sensor": {
    "adcPin": 26,
    "adcRefVoltage": 3.3,
    "sensitivity": 0.0429,
    "offsetVoltage": 1.65
  },
  "advanced": {
    "debugLevel": 2,
    "adaptiveSampling": false,
    "measurementInterval": 1000,
    "dataFormat": "JSON"
  }
}
```

## Software Architecture

Implement the following modular architecture:

1. **Main Application (`AmpMeter_Pico_W.ino`):**
   - Entry point and main loop
   - Handles initialization and coordination between modules
   - Manages timing and scheduling

2. **Configuration Manager (`config_manager.h/cpp`):**
   - Handles loading/saving configuration from/to LittleFS
   - Provides accessors for all configuration parameters
   - Validates configuration changes

3. **LoRaWAN Client (`lorawan_client.h/cpp`):**
   - Manages LoRaWAN communication
   - Handles uplink/downlink
   - Formats data according to specs

4. **BLE Manager (`ble_manager.h/cpp`):**
   - Implements BLE service/characteristics
   - Handles configuration changes via BLE
   - Provides real-time data streaming

5. **WiFi Manager (`wifi_manager.h/cpp`):**
   - Manages WiFi connectivity
   - Handles connection to specified networks
   - Implements error recovery

6. **MQTT Client (`mqtt_client.h/cpp`):**
   - Manages MQTT connection
   - Handles publishing/subscribing
   - Formats data for MQTT transmission

7. **Sensor Manager (`sensor_manager.h/cpp`):**
   - Reads from WCS6800 sensor
   - Applies calibration and filtering
   - Manages adaptive sampling if enabled

8. **OTA Manager (`ota_manager.h/cpp`):**
   - Handles firmware updates
   - Manages update process
   - Validates firmware integrity

## Code Implementation Requirements

### Configuration Manager

Must implement these core methods:
```cpp
bool init();  // Initialize configuration
bool reset(); // Reset to defaults
bool loadFromFile();  // Load from LittleFS
bool saveToFile();  // Save to LittleFS
String getJsonConfig();  // Get full configuration as JSON
bool updateFromJson(String jsonStr);  // Update config from JSON
```

Plus getters and setters for all configuration parameters.

### BLE Manager

Must implement the BLE service with characteristics that:
1. Serialize configuration sections to JSON on read
2. Deserialize and apply JSON configuration on write
3. Implement notifications for real-time data

```cpp
bool init(String deviceName, ConfigManager* configManager);
void handle();  // Process BLE events
void sendData(String data);  // Send sensor data
bool isConnected();  // Check connection status
void updateAllCharacteristics();  // Refresh all characteristics
```

### LoRaWAN Client

Must implement basic LoRaWAN functionality:
```cpp
bool init(String devAddr, String nwksKey, String appsKey, int region);
bool sendData(String payload);  // Send data (will be binary encoded)
bool isConnected();  // Check connection status
void setRegion(int region);  // Change region
bool reset();  // Reset connection
```

### Main Application

Must implement the main loop with these responsibilities:
1. Read sensor at configured intervals
2. Process data according to configuration
3. Transmit data through enabled channels (LoRaWAN, WiFi, BLE)
4. Handle configuration changes from any interface
5. Apply changes immediately when configuration is updated

## Implementation Notes

1. **Default Values:**
   - Use sensible defaults matching Android app expectations
   - Fall back to defaults if configuration is corrupted

2. **Error Handling:**
   - Implement graceful degradation if communication fails
   - Retry failed transmissions with backoff
   - Log errors according to debug level

3. **Memory Considerations:**
   - Optimize JSON processing for limited memory
   - Use static buffers where appropriate
   - Be mindful of string handling on embedded device

4. **Security:**
   - Securely store sensitive credentials
   - Implement authentication for BLE configuration when possible
   - Validate configuration changes

## Testing Guidelines

1. **Configuration Sync Testing:**
   - Verify all parameters correctly sync between app and device
   - Test boundary conditions and invalid values
   - Confirm configuration persists across reboots

2. **Communication Testing:**
   - Test all communication paths (LoRaWAN, BLE, WiFi/MQTT)
   - Verify payloads match expected formats
   - Confirm bidirectional communication works

3. **Integration Testing:**
   - Test complete workflow from app configuration to device behavior
   - Verify all features work together as expected
   - Confirm Android app correctly interprets device data

## Android App Integration Points

The Android app is already set up with these integration points:
1. SettingsRepository - Stores all configuration parameters
2. SettingsViewModel - Manages the UI state and data operations
3. SettingsFragment - UI for configuration
4. BLE Communication - For direct device interaction
5. MQTT Client - For cloud-based configuration and data exchange
6. LoRaWAN integration through ChirpStack

## Required Libraries

- Arduino.h
- SPI.h
- LittleFS.h
- ArduinoJson.h
- BLEDevice.h (for Pico W)
- WiFi.h
- PubSubClient.h (for MQTT)
- LoRaWAN.h (or appropriate LoRaWAN library)
- HTTPClient.h (for OTA updates)

This document provides a comprehensive reference for implementing the synchronization between the LoRaWAN Pico W device and the AmpMeter Android application, ensuring all configuration parameters are properly mapped and communication protocols are correctly implemented.