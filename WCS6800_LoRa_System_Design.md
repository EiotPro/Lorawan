# WCS6800 LoRa Current Sensor System Design

## System Overview

This project implements a current sensing system using a WCS6800 current sensor connected to a Raspberry Pi Pico. The system measures current values and transmits them over LoRaWAN using a RAK3172 module. It also receives downlink commands to control LEDs for visual indication.

## Hardware Components

### Core Components
- **Raspberry Pi Pico**: Microcontroller running MicroPython
- **WCS6800**: AC/DC Current Sensor
- **RAK3172**: LoRaWAN Module

### I/O Connections
- **GPIO 26 (ADC0)**: Connected to WCS6800 VOUT (Analog Input)
- **GPIO 0 (UART0 TX)**: Connected to RAK3172 RX - Blue LED indicator
- **GPIO 1 (UART0 RX)**: Connected to RAK3172 TX
- **GPIO 5 (UART1 RX)**: Red LED indicator
- **GPIO 3**: Status (Green LED indicator)
- **GPIO 2**: RAK3172 Reset pin
- **On-board LED (GPIO 25)**: Used for downlink command visual feedback

### Power Connections
- WCS6800 VCC: Connected to RPi Pico 3.3V
- WCS6800 GND: Connected to RPi Pico GND

## Software Architecture

### MicroPython Component (Pico_Wcs6800.py)

#### Configuration
- **ADC Configuration**: Setup for reading WCS6800 sensor on GPIO 26
- **UART Configuration**: Setup for communication with RAK3172 on UART0 (GPIO 0,1)
- **LoRaWAN ABP Configuration**: Device credentials for ChirpStack

#### Key Functions
1. **read_current()**: Reads and calculates current from the WCS6800 sensor
2. **send_at_command()**: Handles UART communication with the RAK3172
3. **initialize_lorawan()**: Sets up LoRaWAN in ABP mode
4. **send_lorawan_payload()**: Encodes and sends current data via LoRaWAN
5. **listen_for_downlink()**: Processes incoming commands from LoRaWAN network
6. **main()**: Main program loop

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
- **Red LED (GPIO 5/UART1 RX)**: Indicates module readiness and downlink reception
- **Blue LED (GPIO 0/UART0 TX)**: Indicates ongoing data transmission
- **Green LED (GPIO 3)**: System status indicator
- **On-board LED**: Controlled via downlink commands

### LED Behavior Patterns

#### System Startup
- **All LEDs**: Briefly flash once in sequence during initialization
  - Confirms LEDs are functioning properly
  - Visual indication of power-on

#### Red LED Patterns
- **Solid On**: System is waiting for RAK3172 module to be ready
- **Solid On (Later)**: System is listening for downlink messages
- **Rapid Flashing (5 times)**: LoRaWAN initialization failure
- **Rapid Flashing (3 times)**: Error occurred in main loop

#### Blue LED Patterns
- **Solid On**: UART transmission in progress (sending AT command)
- **Off**: No active transmission
- **Automatic Control**: Turns on during LoRaWAN payload transmission and automatically turns off when completed

#### Green LED Patterns
- **Solid On**: During LoRaWAN initialization
- **Stays On**: LoRaWAN network successfully joined
- **Turns Off**: Failed to join LoRaWAN network
- **Pulsing (1s on, 9s off)**: System is in waiting period between transmissions

#### On-board LED Patterns (Controlled via Downlink)
- **Solid On**: Command 0x01 received
- **Off**: Command 0x02 received
- **Triple Blink**: Command 0x04 received

### LED State Summary Table
| Event                           | Red LED        | Blue LED       | Green LED        | On-board LED    |
|---------------------------------|----------------|----------------|------------------|-----------------|
| System Startup                  | Flash once     | Flash once     | Flash once       | Flash once      |
| Waiting for Module              | Solid On       | Off            | Off              | Off             |
| Module Ready                    | Off            | Off            | Off              | Off             |
| LoRaWAN Initialization          | Off            | Off            | Solid On         | Off             |
| AT Command Transmission         | Off            | Solid On       | Depends on state | Off             |
| Network Join Success            | Off            | Off            | Solid On         | Off             |
| Network Join Failure            | Rapid Flashing | Off            | Off              | Off             |
| Payload Transmission            | Off            | Solid On       | Solid On         | Off             |
| Listening for Downlink          | Solid On       | Off            | Solid On         | Off             |
| Wait Period                     | Off            | Off            | Pulsing          | Off             |
| Error in Main Loop              | Rapid Flashing | Off            | Solid On         | Off             |
| Downlink Command Received       | Off            | Off            | Solid On         | Depends on cmd  |

## LoRaWAN Configuration

- **Mode**: ABP (Activation By Personalization)
- **Class**: C (Continuously listening)
- **Region**: IN865 (India)
- **Device Address**: Configured in code
- **Network & Application Session Keys**: Configured in code

## Initialization Sequence

1. Configure WCS6800 ADC input
2. Configure UART for RAK3172 communication
3. Wait for RAK3172 module to be ready
4. Configure LoRaWAN parameters (ABP mode, keys, etc.)
5. Join the LoRaWAN network
6. Begin main measurement and transmission loop

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
- Error reporting via print statements
- Range checking for current values
- Visual error indication via LED patterns

## Future Improvements

- Add support for OTAA (Over The Air Activation)
- Implement power-saving features
- Add more sensor types
- Enhance error logging and recovery mechanisms 