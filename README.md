# LoRaWAN Current Sensor Project

This project implements a WCS6800 current sensor connected to a Raspberry Pi Pico that communicates with a ChirpStack LoRaWAN server using a RAK3172 module.

## Overview

The system measures current values using the WCS6800 sensor, transmits the data over LoRaWAN, and can receive downlink commands to control the onboard LED.

## Components

- **Hardware**: Raspberry Pi Pico, WCS6800 Current Sensor, RAK3172 LoRaWAN Module
- **Software**: Arduino implementation for the Pico, JavaScript codec for ChirpStack

## Key Features

- Current measurement and transmission via LoRaWAN
- Downlink command reception for LED control
- Configurable for different LoRaWAN regions
- ABP (Activation By Personalization) security mode

## Documentation

- **WCS6800_LoRa_System_Design.md**: System architecture and technical details
- **WCS6800_LoRa_Setup_Guide.md**: Step-by-step setup instructions
- **LED_Pin_Configuration.md**: LED pin assignments and usage
- **Debug_Configuration.md**: Debug output configuration

## Getting Started

### Using Arduino IDE

1. Follow the setup guide in WCS6800_LoRa_Setup_Guide.md
2. Configure the ChirpStack server with the provided codec
3. Upload the Arduino code to the Raspberry Pi Pico
4. Monitor the current readings and control the LED via downlink commands

### Using PlatformIO in VS Code

1. Install [VS Code](https://code.visualstudio.com/)
2. Install the [PlatformIO extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)
3. Open this project folder in VS Code
4. PlatformIO will automatically detect the configuration in `platformio.ini`
5. Use the PlatformIO sidebar to build and upload the project
6. Use the PlatformIO Serial Monitor to view debug output

## Project Structure

```
Lorawan/
├── platformio.ini          # PlatformIO configuration
├── src/
│   └── main.cpp            # Main Arduino code
├── Chirpstack_codec.js     # ChirpStack codec for data encoding/decoding
├── Debug_Configuration.md  # Debug output documentation
├── LED_Pin_Configuration.md # LED configuration documentation
├── README.md               # This file
├── WCS6800_LoRa_Setup_Guide.md # Setup instructions
└── WCS6800_LoRa_System_Design.md # System architecture documentation
```

## License

This project is released under the MIT license.
