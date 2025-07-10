# WCS6800 LoRa Current Sensor System Design

## System Overview

This project implements a current sensing system using a WCS6800 current sensor connected to a Raspberry Pi Pico. The system measures current values and transmits them over LoRaWAN using a RAK3172 module. It also receives downlink commands to control LEDs for visual indication.

## Hardware Components

### Core Components
- **Raspberry Pi Pico**: Microcontroller running Arduino
- **WCS6800**: AC/DC Current Sensor
- **RAK3172**: LoRaWAN Module

### I/O Connections
- **GPIO 26 (ADC0)**: Connected to WCS6800 VOUT (Analog Input)
- **GPIO 0 (UART0 TX)**: Connected to RAK3172 RX
- **GPIO 1 (UART0 RX)**: Connected to RAK3172 TX
- **GPIO 25**: On-board LED used for downlink command visual feedback
- **GPIO 30**: RAK3172 Reset pin

### Power Connections
- WCS6800 VCC: Connected to RPi Pico 3.3V
- WCS6800 GND: Connected to RPi Pico GND

## Software Architecture

### Arduino Component (Pico_WCS6800.ino)

#### Configuration
- **ADC Configuration**: Setup for reading WCS6800 sensor on GPIO 26
- **UART Configuration**: Setup for communication with RAK3172 on Software Serial (GPIO 0,1)
- **LoRaWAN ABP Configuration**: Device credentials for ChirpStack

#### Key Functions
1. **readCurrent()**: Reads and calculates current from the WCS6800 sensor
2. **sendATCommand()**: Handles UART communication with the RAK3172
3. **initializeLoRaWAN()**: Sets up LoRaWAN in ABP mode
4. **sendLoRaWANPayload()**: Encodes and sends current data via LoRaWAN
5. **listenForDownlink()**: Processes incoming commands from LoRaWAN network
6. **setup() and loop()**: Standard Arduino program structure

#### Data Flow
1. Read analog value from WCS6800 sensor
2. Convert to meaningful current value
3. Encode as 2-byte payload
4. Send via LoRaWAN using RAK3172
5. Listen for downlink commands
6. Process commands to control LED

### ChirpStack Component (Chirpstack_codec.js)

#### Decoder Functions
- **decodeUplink()**: Parses incoming binary data from device
  - Converts 2-byte signed integer to current value
  - Adds metadata and status information

#### Encoder Functions
- **encodeDownlink()**: Creates binary commands to send to device
  - Supports LED commands: on, off, blink
  - Supports raw command values

#### Test Functions
- **testDecoder()**: Tests the uplink decoding with sample payloads
- **testEncoder()**: Tests the downlink encoding with sample commands

## Communication Protocol

### Uplink (Device to Server)
- **Payload Format**: 2-byte signed integer (big-endian)
- **Value**: Current in milliamperes (mA)
- **Port**: 2
- **Frequency**: Every 60 seconds

### Downlink (Server to Device)
- **Payload Format**: 1-byte command
- **Commands**:
  - 0x01: Turn LED ON
  - 0x02: Turn LED OFF
  - 0x04: Blink LED

## LED Indicators

### LED Physical Assignment
- **GPIO 25 (On-board LED)**: Controlled via downlink commands

### LED Behavior Patterns

#### System Startup
- System initialization information displayed on serial monitor

#### On-board LED Patterns (Controlled via Downlink)
- **Solid On**: Command 0x01 received
- **Off**: Command 0x02 received
- **Triple Blink**: Command 0x04 received

## LoRaWAN Configuration

- **Mode**: ABP (Activation By Personalization)
- **Class**: C (Continuously listening)
- **Region**: IN865 (India)
- **Device Address**: Configured in code
- **Network & Application Session Keys**: Configured in code

## Initialization Sequence

1. Initialize serial communication
2. Configure GPIO pins
3. Set ADC resolution to 12-bit
4. Wait for RAK3172 module to be ready
5. Configure LoRaWAN parameters (ABP mode, keys, etc.)
6. Join the LoRaWAN network
7. Begin main measurement and transmission loop

## Main Operation Loop

1. Read current value from WCS6800 sensor
2. Format and send data via LoRaWAN
3. Listen for downlink commands
4. Process any received commands
5. Wait 60 seconds
6. Repeat

## Error Handling

- UART communication timeouts
- Module initialization retry mechanism
- Error reporting via serial monitor
- Range checking for current values

## Future Improvements

- Add support for OTAA (Over The Air Activation)
- Implement power-saving features
- Add more sensor types
- Enhance error logging and recovery mechanisms 