# WCS6800 LoRaWAN Current Sensor - Operational Manual

## System Overview

The WCS6800 LoRaWAN Current Sensor is a comprehensive IoT solution built on the Raspberry Pi Pico W platform. It combines LoRaWAN, WiFi, and Bluetooth connectivity to provide multiple communication channels for current sensing data and remote control. This system is ideal for industrial monitoring, energy management, and remote equipment supervision.

## Key Features

- **Multiple Connectivity Options**:
  - LoRaWAN for long-range, low-power communication
  - WiFi for local high-bandwidth data transfer and OTA updates
  - Bluetooth for direct device interaction and monitoring
- **Remote Monitoring and Control**:
  - Real-time current measurements with 12-bit resolution
  - MQTT publishing for dashboard integration
  - BLE services for smartphone connectivity
  - Downlink commands for remote control
- **Maintenance and Updates**:
  - Over-the-Air (OTA) firmware updates
  - Comprehensive logging and diagnostics
  - Error handling and automatic recovery
- **Modular Architecture**:
  - Configurable via `config.h` file
  - Separate functional modules for each feature

## Hardware Requirements

- **Raspberry Pi Pico W** (required for WiFi/BLE functionality)
- **WCS6800 Current Sensor**
- **RAK3172 LoRaWAN Module**
- **Connecting wires and breadboard**
- **USB cable for power and programming**

## System Architecture

The system is built with a modular architecture to ensure maintainability and extensibility:

```
┌──────────────────────────────────────────────┐
│                  main.cpp                    │
│  (Core application logic and orchestration)  │
└───────────────┬──────────────┬──────────────┘
                │              │
┌───────────────▼──┐ ┌─────────▼────────┐ ┌────────────────────┐
│    utils.cpp     │ │   config.h       │ │  Hardware Layer    │
│ (Helper functions)│ │(Configuration)   │ │(Sensors/Actuators)│
└──────────────────┘ └─────────┬────────┘ └────────────────────┘
                               │
      ┌────────────────────────┼────────────────────────┐
      │                        │                        │
┌─────▼─────┐           ┌──────▼──────┐          ┌──────▼──────┐
│LoRaWAN    │           │WiFi/MQTT    │          │Bluetooth    │
│(Long range)│           │(Local network)│        │(Direct conn.)│
└───────────┘           └──────────────┘          └──────────────┘
```

## Initial Setup

### Hardware Connections

1. **WCS6800 Current Sensor**:
   - VCC → Pico W 3.3V
   - GND → Pico W GND
   - VOUT → Pico W GPIO 26 (ADC0)

2. **RAK3172 LoRaWAN Module**:
   - VCC → Pico W 3.3V
   - GND → Pico W GND
   - TX → Pico W GPIO 1 (UART0 RX)
   - RX → Pico W GPIO 0 (UART0 TX)
   - RST → Pico W GPIO 2

3. **On-board Components**:
   - LED → GPIO 25 (on-board LED)

### Software Configuration

1. **Update config.h**:
   - Set appropriate debug level
   - Configure LoRaWAN credentials
   - Set WiFi/MQTT parameters if using
   - Enable/disable features as needed

2. **Compile and Upload**:
   - Using PlatformIO:
     ```
     pio run -t upload -e rpipicow
     ```
   - Monitor output:
     ```
     pio device monitor -b 115200
     ```

## Operational Modes

### Basic Mode (LoRaWAN Only)

In this mode, only LoRaWAN functionality is enabled:

1. **Enable in config.h**:
   ```cpp
   #define WIFI_ENABLED false
   #define BLE_ENABLED false
   #define OTA_ENABLED false
   ```

2. **Operation**:
   - System reads current values from WCS6800
   - Data is transmitted via LoRaWAN every TX_INTERVAL
   - System listens for downlink commands
   - LED is controlled based on received commands

### Enhanced Mode (WiFi + LoRaWAN)

Adding WiFi enables local connectivity and MQTT integration:

1. **Enable in config.h**:
   ```cpp
   #define WIFI_ENABLED true
   #define MQTT_ENABLED true
   const char* const WIFI_SSID = "YourNetwork";
   const char* const WIFI_PASSWORD = "YourPassword";
   const char* const MQTT_BROKER = "192.168.1.100";
   ```

2. **Operation**:
   - All Basic Mode functionality
   - WiFi connection to local network
   - Current data published to MQTT broker
   - More frequent updates possible via WiFi (WIFI_TX_INTERVAL)

### Full Mode (WiFi + BLE + LoRaWAN + OTA)

All features enabled for maximum functionality:

1. **Enable in config.h**:
   ```cpp
   #define WIFI_ENABLED true
   #define BLE_ENABLED true
   #define OTA_ENABLED true
   ```

2. **Operation**:
   - All Enhanced Mode functionality
   - BLE services for direct device connection
   - OTA updates via WiFi when triggered
   - Multi-channel data publishing

## Communication Protocols

### LoRaWAN

- **Mode**: ABP (Activation By Personalization)
- **Class**: Class C (continuous reception)
- **Region**: Configurable (default: IN865)
- **Data Format**:
  - Uplink: 2 bytes (signed int, current in mA)
  - Downlink: 1 byte command codes

### WiFi/MQTT

- **WiFi**: Standard 2.4GHz connection
- **MQTT Topics**:
  - Publish: "wcs6800/data" (configurable)
  - Data Format: JSON with current reading and timestamp
  - Example: `{"current":0.125,"timestamp":1234567,"unit":"A"}`

### Bluetooth

- **BLE Services**:
  - Current Value (readable, notifiable)
  - Command Service (writable)
- **Commands**:
  - "LED_ON" - Turn on LED
  - "LED_OFF" - Turn off LED
  - "BLINK_LED" - Blink LED pattern

## Remote Control Features

### LoRaWAN Downlink Commands

Commands sent via LoRaWAN downlink:

| Payload | Function          | Response                  |
|---------|-------------------|---------------------------|
| 01      | Turn LED ON       | LED illuminates           |
| 02      | Turn LED OFF      | LED turns off             |
| 03      | Trigger OTA update| System downloads firmware |
| 04      | Blink LED         | LED blinks three times    |

### MQTT Control

Future versions will support MQTT command subscription for remote control.

### BLE Control

Immediate control via BLE service:

1. Connect to device via BLE (device name: WCS6800_Monitor)
2. Write to Command characteristic
3. Commands: "LED_ON", "LED_OFF", "BLINK_LED"

## Over-the-Air Updates

The system supports OTA firmware updates via WiFi:

1. **Preparation**:
   - Host firmware binary on HTTP server
   - Configure server URL in config.h:
     ```cpp
     const char* const OTA_SERVER_URL = "http://server/firmware.bin";
     ```

2. **Trigger Update**:
   - Send LoRaWAN downlink command "03"
   - System connects to WiFi
   - Downloads firmware from configured server
   - Verifies and applies update
   - Reboots with new firmware

## Diagnostic and Monitoring

### Serial Monitoring

Connect via USB and monitor at 115200 baud:

```
pio device monitor -b 115200
```

### Log Levels

Configure log verbosity in config.h:

```cpp
#define DEBUG_LEVEL 2  // 0=OFF, 1=ERROR, 2=INFO, 3=DEBUG, 4=VERBOSE
```

### MQTT Monitoring

Use MQTT client to subscribe to device topic:

```
mosquitto_sub -h <broker_ip> -t wcs6800/data
```

## Maintenance Procedures

### Firmware Updates

1. **OTA Update**:
   - Ensure WiFi and OTA are enabled
   - Host new firmware on HTTP server
   - Send downlink command "03"
   - Monitor update progress via serial

2. **USB Update**:
   - Connect via USB
   - Upload using PlatformIO:
     ```
     pio run -t upload -e rpipicow
     ```

### Calibration

For improved accuracy, calibrate the WCS6800 sensor:

1. Ensure no current is flowing
2. Measure output voltage at 0A
3. Update config.h with measured offset:
   ```cpp
   #define WCS6800_OFFSET_VOLTAGE 1.65 // Replace with measured value
   ```
4. Apply known current, measure, and adjust sensitivity:
   ```cpp
   #define WCS6800_SENSITIVITY 0.0429 // Adjust as needed
   ```

### Diagnostics

Run full diagnostic check on system:

```cpp
if (!runFullDiagnostics()) {
  log_error("Hardware diagnostics failed, check components");
}
```

## Troubleshooting

### Common Issues

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No serial output | Power issue or corrupt firmware | Check USB connection, re-upload firmware |
| WiFi won't connect | Wrong credentials or out of range | Verify SSID/password in config.h |
| LoRaWAN join failure | Incorrect credentials or gateway issue | Check ABP credentials, verify gateway operation |
| Inaccurate readings | WCS6800 calibration | Recalibrate sensor offset and sensitivity |
| OTA update fails | WiFi issues or server unreachable | Check WiFi connection and server URL |

### Reset Procedure

If the system becomes unresponsive:

1. Disconnect power
2. Hold BOOTSEL button
3. Reconnect power
4. Re-upload firmware

## Appendices

### Configuration Reference

See config.h for all configurable parameters:

- DEBUG_LEVEL: Log verbosity
- ADC parameters for WCS6800 sensor
- UART settings for RAK3172
- LoRaWAN credentials
- WiFi/MQTT settings
- Bluetooth settings
- OTA update settings

### Pin Assignment Reference

| GPIO Pin | Function | Connected To |
|----------|----------|-------------|
| 0 (UART TX) | UART Output | RAK3172 RX |
| 1 (UART RX) | UART Input | RAK3172 TX |
| 2 | Reset Line | RAK3172 RST |
| 25 | LED Output | On-board LED |
| 26 (ADC0) | Analog Input | WCS6800 VOUT |

### Power Consumption

| Mode | Average Current | Battery Life (2000mAh) |
|------|----------------|------------------------|
| LoRaWAN only | ~30mA | ~66 hours |
| With WiFi active | ~100mA | ~20 hours |
| With BLE active | ~50mA | ~40 hours |
| Sleep mode | ~5mA | ~400 hours | 