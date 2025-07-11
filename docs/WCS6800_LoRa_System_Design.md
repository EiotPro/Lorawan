# WCS6800 LoRa Current Sensor System Design

## System Overview

This project implements a current sensing system using a WCS6800 current sensor connected to a Raspberry Pi Pico W. The system measures current values and transmits them over LoRaWAN using a RAK3172 module. Additionally, it provides WiFi and Bluetooth connectivity for local data access and supports Over-the-Air (OTA) firmware updates. The system receives downlink commands to control LEDs and trigger OTA updates.

## Hardware Components

### Core Components
- **Raspberry Pi Pico W**: Microcontroller with WiFi/BLE capabilities
- **WCS6800**: AC/DC Current Sensor
- **RAK3172**: LoRaWAN Module

### I/O Connections
- **GPIO 26 (ADC0)**: Connected to WCS6800 VOUT (Analog Input)
- **GPIO 0 (UART0 TX)**: Connected to RAK3172 RX
- **GPIO 1 (UART0 RX)**: Connected to RAK3172 TX
- **GPIO 25**: On-board LED used for downlink command visual feedback
- **GPIO 2**: RAK3172 Reset pin

### Power Connections
- WCS6800 VCC: Connected to RPi Pico 3.3V
- WCS6800 GND: Connected to RPi Pico GND

## Software Architecture

The software follows a modular design pattern with clearly separated components:

```
┌─────────────────┐      ┌────────────────┐      ┌────────────────┐
│    main.cpp     │◄────►│    config.h    │      │    utils.h     │
│ (Core Logic)    │      │(Configuration) │      │(Utility Funcs) │
└────────┬────────┘      └────────────────┘      └────────┬───────┘
         │                                               │
         │                                               │
         ▼                                               ▼
┌─────────────────┐      ┌────────────────┐      ┌────────────────┐
│ wifi_manager.h  │      │ ble_manager.h  │      │ ota_manager.h  │
│ (WiFi & MQTT)   │      │(Bluetooth Low E)│      │(OTA Updates)   │
└─────────────────┘      └────────────────┘      └────────────────┘
```

### Core Components (main.cpp)

#### Configuration
- **ADC Configuration**: Setup for reading WCS6800 sensor on GPIO 26
- **UART Configuration**: Setup for communication with RAK3172 on UART (GPIO 0,1)
- **LoRaWAN ABP Configuration**: Device credentials for ChirpStack
- **WiFi/MQTT Configuration**: Local network connectivity
- **BLE Configuration**: Bluetooth services and characteristics
- **OTA Configuration**: Over-the-air update settings

#### Key Functions
1. **readCurrent()**: Reads and calculates current from the WCS6800 sensor
2. **sendATCommand()**: Handles UART communication with the RAK3172
3. **initializeLoRaWAN()**: Sets up LoRaWAN in ABP mode
4. **sendLoRaWANPayload()**: Encodes and sends current data via LoRaWAN
5. **listenForDownlink()**: Processes incoming commands from LoRaWAN network
6. **setup() and loop()**: Standard Arduino program structure

### WiFi/MQTT Component (wifi_manager.cpp)

#### Key Functions
1. **setupWiFi()**: Initializes WiFi connectivity
2. **connectToWiFi()**: Manages WiFi connection with retry logic
3. **setupMQTT()**: Configures MQTT client
4. **publishCurrentData()**: Sends current readings to MQTT broker
5. **handleWiFiEvents()**: Manages WiFi state and reconnection

### Bluetooth Component (ble_manager.cpp)

#### Key Functions
1. **setupBLE()**: Initializes Bluetooth services
2. **updateBLECurrentValue()**: Updates characteristic with new readings
3. **handleBLEEvents()**: Manages BLE connections and notifications
4. **processBLECommand()**: Handles commands received via BLE

### OTA Update Component (ota_manager.cpp)

#### Key Functions
1. **setupOTA()**: Initializes OTA capabilities
2. **triggerOTAUpdate()**: Starts OTA process when triggered
3. **downloadAndApplyOTA()**: Downloads and installs firmware
4. **handleOTAEvents()**: Manages OTA state machine

### Utility Component (utils.cpp)

#### Key Functions
1. **log_xxx()**: Different levels of logging functions
2. **checkWCS6800Sensor()**: Diagnostic for current sensor
3. **checkRAK3172Module()**: Diagnostic for LoRa module
4. **handleError()**: Error management and recovery
5. **formatCurrentAsJson()**: Creates JSON data for WiFi/BLE transmission

## Communication Protocols

### LoRaWAN
- **Payload Format**: 2-byte signed integer (big-endian)
- **Value**: Current in milliamperes (mA)
- **Port**: 2
- **Frequency**: Every TX_INTERVAL (configurable, default 60 seconds)

### WiFi/MQTT
- **Connection**: Standard WiFi to local network
- **Protocol**: MQTT publish/subscribe
- **Topics**: 
  - Publish: wcs6800/data (configurable)
- **Data Format**: JSON with current and timestamp
- **Frequency**: Every WIFI_TX_INTERVAL (configurable)

### Bluetooth
- **Protocol**: Bluetooth Low Energy
- **Services**:
  - Current Data Service: Read current values
  - Command Service: Control device functions
- **Frequency**: Every BLE_TX_INTERVAL (configurable)

## Downlink Commands

### LoRaWAN Downlink
- **Command 0x01**: Turn LED ON
- **Command 0x02**: Turn LED OFF
- **Command 0x03**: Trigger OTA update
- **Command 0x04**: Blink LED

### BLE Commands
- **LED_ON**: Turn LED ON
- **LED_OFF**: Turn LED OFF
- **BLINK_LED**: Blink LED pattern

## LED Indicators

### LED Physical Assignment
- **GPIO 25 (On-board LED)**: Controlled via downlink commands or BLE

### LED Behavior Patterns

#### System Startup
- System initialization information displayed on serial monitor

#### On-board LED Patterns
- **Solid On**: Command 0x01 received
- **Off**: Command 0x02 received
- **Triple Blink**: Command 0x04 received

## LoRaWAN Configuration

- **Mode**: ABP (Activation By Personalization)
- **Class**: C (Continuously listening)
- **Region**: IN865 (India) - configurable
- **Device Address**: Configured in code
- **Network & Application Session Keys**: Configured in code

## WiFi/MQTT Configuration

- **SSID/Password**: Configured in code
- **MQTT Broker**: IP address/hostname and port configured in code
- **MQTT Authentication**: Username/password if required
- **Topic**: Configurable publish topic

## BLE Configuration

- **Device Name**: "WCS6800_Monitor" (configurable)
- **Services**: Current value and command services
- **Security**: Standard BLE security

## OTA Update System

- **Trigger**: LoRaWAN downlink command 0x03
- **Transport**: WiFi connection to HTTP server
- **Server**: Configurable URL
- **Authentication**: Optional HTTP basic auth
- **Verification**: Firmware integrity check before applying

## Initialization Sequence

1. Initialize serial communication
2. Configure GPIO pins
3. Set ADC resolution to 12-bit
4. Initialize WiFi, BLE, and OTA if enabled
5. Wait for RAK3172 module to be ready
6. Configure LoRaWAN parameters (ABP mode, keys, etc.)
7. Join the LoRaWAN network
8. Begin main measurement and transmission loop

## Main Operation Loop

1. Handle WiFi, BLE, and OTA events
2. Read current value from WCS6800 sensor
3. Update BLE characteristic if connected
4. Publish to MQTT if enabled
5. Format and send data via LoRaWAN at configured intervals
6. Listen for downlink commands
7. Process any received commands
8. Continue the loop

## Error Handling

- Comprehensive error code system
- UART communication timeouts
- WiFi connection retry mechanism
- Module initialization retry mechanism
- Error reporting via serial monitor
- Range checking for current values
- Error recovery procedures

## Future Improvements

- Add support for OTAA (Over The Air Activation)
- Implement power-saving features
- Extend BLE services for more functionality
- Add more sensor types
- Implement bidirectional MQTT control
- Create mobile app for BLE interaction 