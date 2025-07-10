# LoRaWAN Current Sensor Project with WiFi/BLE Connectivity

This project implements a WCS6800 current sensor connected to a Raspberry Pi Pico W that communicates with a ChirpStack LoRaWAN server using a RAK3172 module. The project leverages the Pico W's WiFi and Bluetooth capabilities for local data monitoring and Over-the-Air (OTA) updates.

## Overview

The system measures current values using the WCS6800 sensor, transmits the data over LoRaWAN, and provides additional connectivity via WiFi and Bluetooth. It also supports OTA firmware updates triggered by LoRaWAN downlink commands.

## Components

- **Hardware**: Raspberry Pi Pico W, WCS6800 Current Sensor, RAK3172 LoRaWAN Module
- **Software**: Arduino implementation for the Pico W with WiFi, BLE, and OTA capabilities

## Key Features

- Current measurement and transmission via LoRaWAN
- WiFi connectivity for local data monitoring via MQTT
- Bluetooth connectivity for local data monitoring and control
- Over-the-Air (OTA) firmware updates via WiFi
- Downlink command reception for LED control and OTA triggering
- Configurable for different LoRaWAN regions
- ABP (Activation By Personalization) security mode
- Modular code structure with utilities for debugging and maintenance

## Documentation

- **WCS6800_LoRa_System_Design.md**: System architecture and technical details
- **WCS6800_LoRa_Setup_Guide.md**: Step-by-step setup instructions
- **LED_Pin_Configuration.md**: LED pin assignments and usage
- **Debug_Configuration.md**: Debug output configuration

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

- **Debug Level**: Set verbosity of logs
- **WiFi**: Enable/disable WiFi, set SSID, password, and transmission interval
- **MQTT**: Configure broker details for data publishing
- **Bluetooth**: Enable/disable BLE, set device name and transmission interval
- **OTA Updates**: Configure update server and credentials

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
│   └── WCS6800_LoRa_System_Design.md # System architecture documentation
└── README.md               # This file
```

## WiFi/MQTT Integration

The system can connect to a local WiFi network and publish current readings to an MQTT broker. This allows integration with home automation systems, custom dashboards, or IoT platforms.

## Bluetooth Integration

Bluetooth connectivity enables local monitoring via a smartphone app or any BLE-capable device. The device provides a readable characteristic for current values and a writable characteristic for sending commands.

## OTA Updates

Over-the-Air updates can be triggered via a LoRaWAN downlink command (payload `03`). The system will download the firmware from the configured server and apply it automatically.

## License

This project is released under the MIT license.
