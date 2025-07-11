# LoRaWAN Current Sensor Project with WiFi/BLE Connectivity

This project implements a WCS6800 current sensor connected to a Raspberry Pi Pico W that communicates with a ChirpStack LoRaWAN server using a RAK3172 module. The project leverages the Pico W's WiFi and Bluetooth capabilities for local data monitoring and Over-the-Air (OTA) updates.

## Overview

The system measures current values using the WCS6800 sensor, transmits the data over LoRaWAN, and provides additional connectivity via WiFi and Bluetooth. It also supports OTA firmware updates triggered by LoRaWAN downlink commands.

## Components

- **Hardware**: Raspberry Pi Pico W, WCS6800 Current Sensor, RAK3172 LoRaWAN Module
- **Software**: Arduino implementation with Earlephilhower core for proper Pico W support

## Key Features

- Current measurement and transmission via LoRaWAN
- WiFi connectivity for local data monitoring via MQTT
- Bluetooth Low Energy support (limited due to Earlephilhower core constraints)
- Over-the-Air (OTA) firmware updates via WiFi
- Downlink command reception for LED control and OTA triggering
- Configurable for different LoRaWAN regions
- ABP (Activation By Personalization) security mode
- Modular code structure with comprehensive logging and utilities
- millis()-based timing for non-blocking operations

## Documentation

- **WCS6800_LoRaWAN_Operational_Manual.md**: Comprehensive operational guide
- **WCS6800_LoRa_System_Design.md**: System architecture and technical details
- **WCS6800_LoRa_Setup_Guide.md**: Step-by-step setup instructions
- **LED_Pin_Configuration.md**: LED pin assignments and usage
- **Debug_Configuration.md**: Debug output configuration
- **TEST_INSTRUCTIONS.md**: Phased testing approach for the system

## Getting Started

### Using PlatformIO in VS Code

1. Install [VS Code](https://code.visualstudio.com/)
2. Install the [PlatformIO extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)
3. Open this project folder in VS Code
4. PlatformIO will automatically detect the configuration in `platformio.ini`
5. Use the PlatformIO sidebar to build and upload the project
6. Use the PlatformIO Serial Monitor to view debug output

### Configuration

The system is highly configurable through the `config.h` file:

- **Debug Level**: Set verbosity of logs (0=OFF, 1=ERROR, 2=INFO, 3=DEBUG, 4=VERBOSE)
- **WiFi**: Enable/disable WiFi, set SSID, password, and transmission interval
- **MQTT**: Configure broker details for data publishing
- **Bluetooth**: Enable/disable BLE, set device name and transmission interval
- **OTA Updates**: Configure update server and credentials
- **LoRaWAN**: Configure device address, network and application session keys

## Project Structure

```
Lorawan/
├── platformio.ini          # PlatformIO configuration
├── include/                # Header files
│   ├── config.h            # Configuration parameters
│   ├── utils.h             # Utility functions
│   ├── wifi_manager.h      # WiFi/MQTT functionality
│   ├── ble_manager.h       # Bluetooth functionality
│   └── ota_manager.h       # OTA update functionality
├── src/                    # Source files
│   ├── main.cpp            # Main application code
│   ├── utils.cpp           # Utility function implementations
│   ├── wifi_manager.cpp    # WiFi/MQTT implementations
│   ├── ble_manager.cpp     # Bluetooth implementations
│   └── ota_manager.cpp     # OTA update implementations
├── docs/                   # Documentation
│   ├── Chirpstack_codec.js # ChirpStack codec for data encoding/decoding
│   ├── Debug_Configuration.md  # Debug output documentation
│   ├── LED_Pin_Configuration.md # LED configuration documentation
│   ├── WCS6800_LoRa_Setup_Guide.md # Setup instructions
│   ├── WCS6800_LoRa_System_Design.md # System architecture documentation
│   └── WCS6800_LoRaWAN_Operational_Manual.md # Comprehensive operational guide
├── TEST_INSTRUCTIONS.md    # Testing procedures
└── README.md               # This file
```

## Implementation Details

### LoRaWAN Communication

- **Protocol**: LoRaWAN 1.0.3
- **Mode**: ABP (Activation By Personalization)
- **Class**: Class C (continuous reception)
- **Payload Format**: 2-byte signed integer for current in milliamperes
- **Downlink Commands**: LED control, OTA trigger, custom actions

### WiFi/MQTT Integration

The system connects to a local WiFi network and publishes current readings to an MQTT broker in JSON format. This allows integration with:
- Home automation systems (Home Assistant, OpenHAB)
- Custom dashboards (Node-RED, Grafana)
- IoT platforms (AWS IoT, Azure IoT)

### Bluetooth Integration

Bluetooth Low Energy functionality provides:
- Local monitoring via BLE client apps
- Current value readings through a readable characteristic
- Command execution via a writable characteristic

Note: BLE functionality is limited with the Earlephilhower core and uses stub implementations where needed.

### OTA Updates

Over-the-Air updates can be triggered via:
- LoRaWAN downlink command (payload `03`)
- Potential future support for BLE-triggered updates

The system downloads firmware from the configured HTTP server and applies it automatically.

## Phased Testing Approach

The system is designed to be tested in phases:
1. **Phase 1**: Basic LoRaWAN functionality
2. **Phase 2**: WiFi and MQTT functionality
3. **Phase 3**: OTA update capability

See TEST_INSTRUCTIONS.md for detailed testing procedures.

## Known Limitations

- BLE functionality is limited due to the Earlephilhower core's current BLE support
- WiFi and BLE significantly increase power consumption
- Multiple concurrent communication methods may affect timing

## License

This project is released under the MIT license.
