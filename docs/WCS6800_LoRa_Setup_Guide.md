# WCS6800 LoRa Current Sensor - Setup Guide

## Prerequisites

Before setting up the system, ensure you have the following:

1. Raspberry Pi Pico W (required for WiFi/BLE functionality)
2. WCS6800 Current Sensor
3. RAK3172 LoRaWAN Module
4. LED (optional for visual feedback, on-board LED is used by default)
5. Jumper wires and breadboard
6. Micro USB cable for programming and power
7. Access to a ChirpStack LoRaWAN server and gateway
8. WiFi network for local connectivity (if using WiFi features)
9. MQTT broker (if using MQTT functionality)
10. HTTP server (if using OTA updates)

## Hardware Assembly

### Step 1: Connect the WCS6800 Current Sensor

1. Connect WCS6800 VCC to Raspberry Pi Pico W 3.3V
2. Connect WCS6800 GND to Raspberry Pi Pico W GND
3. Connect WCS6800 VOUT to Raspberry Pi Pico W GPIO 26 (ADC0)

### Step 2: Connect the RAK3172 LoRaWAN Module

1. Connect RAK3172 VCC to Raspberry Pi Pico W 3.3V
2. Connect RAK3172 GND to Raspberry Pi Pico W GND
3. Connect RAK3172 TX to Raspberry Pi Pico W GPIO 1 (UART0 RX)
4. Connect RAK3172 RX to Raspberry Pi Pico W GPIO 0 (UART0 TX)
5. Connect RAK3172 RESET to Raspberry Pi Pico W GPIO 2

### Step 3: Connect LED Indicator (Optional)

The on-board LED (GPIO 25) is used for visual feedback from downlink commands. No additional connection is required for this LED.

## Software Setup

### Step 1: Set Up PlatformIO

1. Install Visual Studio Code from [code.visualstudio.com](https://code.visualstudio.com/)
2. Install the PlatformIO extension from the VS Code marketplace
3. Clone or download the project repository
4. Open the project folder in VS Code with PlatformIO

### Step 2: Configure the Raspberry Pi Pico W Board

Ensure your platformio.ini file contains the correct settings:

```ini
[env]
platform = https://github.com/maxgerhardt/platform-raspberrypi.git
framework = arduino
board_build.core = earlephilhower
monitor_speed = 115200

[env:rpipicow]
board = rpipicow

; Build options
build_flags = 
    -D ARDUINO_ARCH_RP2040
    -D DEBUG_MODE=1
    -D PIN_SERIAL1_TX=0
    -D PIN_SERIAL1_RX=1

; Libraries
lib_deps =
    ; MQTT client
    knolleary/PubSubClient@^2.8
    ; ArduinoJson for JSON formatting
    bblanchon/ArduinoJson@^6.21.3
```

### Step 3: Set Up ChirpStack Server

1. Register a new device in ChirpStack with ABP activation
2. Note down the following credentials:
   - Device Address
   - Network Session Key
   - Application Session Key
3. Ensure the codec functions are applied to your application in ChirpStack:
   - Upload the `docs/Chirpstack_codec.js` file to your ChirpStack application

### Step 4: Configure Project Settings

1. Open the `include/config.h` file and update:

   a. LoRaWAN settings:
   ```cpp
   #define LORAWAN_DEV_ADDR "01d3257c"  // Replace with your Device Address
   #define LORAWAN_NWKS_KEY "06ebd62a3b4e2ed8d45d38d0f515988e"  // Replace with your Network Session Key
   #define LORAWAN_APPS_KEY "ef54ccd9b3d974e8736c60d916ad6e96"  // Replace with your Application Session Key
   #define LORAWAN_REGION 3  // Change to your region's code
   ```

   b. WiFi settings (if using WiFi):
   ```cpp
   #define WIFI_ENABLED true
   const char* const WIFI_SSID = "YourNetworkName";
   const char* const WIFI_PASSWORD = "YourNetworkPassword";
   ```

   c. MQTT settings (if using MQTT):
   ```cpp
   #define MQTT_ENABLED true
   const char* const MQTT_BROKER = "192.168.1.100";  // Your MQTT broker IP
   #define MQTT_PORT 1883
   const char* const MQTT_USERNAME = "user";  // If authentication is needed
   const char* const MQTT_PASSWORD = "password";  // If authentication is needed
   const char* const MQTT_TOPIC = "wcs6800/data";
   ```

   d. OTA settings (if using OTA updates):
   ```cpp
   #define OTA_ENABLED true
   const char* const OTA_SERVER_URL = "http://192.168.1.100:8080/firmware.bin";
   const char* const OTA_HTTP_USERNAME = "admin";  // If authentication is needed
   const char* const OTA_HTTP_PASSWORD = "admin";  // If authentication is needed
   ```

   e. Bluetooth settings (if using BLE):
   ```cpp
   #define BLE_ENABLED true
   const char* const BLE_DEVICE_NAME = "WCS6800_Monitor";
   ```

   f. Debug level:
   ```cpp
   #define DEBUG_LEVEL 2  // 0=OFF, 1=ERROR, 2=INFO, 3=DEBUG, 4=VERBOSE
   ```

### Step 5: Upload Code to Raspberry Pi Pico W

1. Connect the Raspberry Pi Pico W to your computer via USB
2. Press and hold the BOOTSEL button while connecting to enter bootloader mode
3. In VS Code with PlatformIO:
   - Click the PlatformIO icon in the sidebar
   - Select "Upload" under the "Project Tasks" menu
   - Wait for the compilation and upload to complete
4. Once uploaded, the Pico W will restart and run the program automatically

## Testing the System

### Phased Testing Approach

We recommend testing the system in phases:

1. **Phase 1**: Test basic LoRaWAN functionality
2. **Phase 2**: Test WiFi and MQTT (if enabled)
3. **Phase 3**: Test BLE functionality (if enabled)
4. **Phase 4**: Test OTA updates (if enabled)

### Phase 1: Basic LoRaWAN Functionality

1. Connect to the serial monitor at 115200 baud:
   ```
   pio device monitor -b 115200
   ```
2. Verify:
   - System initialization messages
   - Successful LoRaWAN connection
   - Current readings and LoRaWAN transmissions
   - Check that readings appear in ChirpStack

### Phase 2: WiFi and MQTT Functionality

1. Enable WiFi and MQTT in config.h
2. Upload the firmware
3. Verify:
   - WiFi connection success messages
   - MQTT connection success messages
   - Use an MQTT client to subscribe to your topic:
     ```
     mosquitto_sub -h <broker_ip> -t wcs6800/data -v
     ```
   - Verify that data is received by the MQTT broker

### Phase 3: BLE Functionality

1. Enable BLE in config.h
2. Upload the firmware
3. Use a BLE scanner app on your smartphone:
   - Scan for "WCS6800_Monitor" device
   - Connect to the device
   - View current readings
   - Send commands if your app supports BLE writes

### Phase 4: OTA Updates

1. Enable OTA in config.h
2. Upload the firmware
3. Prepare a new firmware binary:
   ```
   pio run -t buildprog
   ```
   Find the .bin file in .pio/build/rpipicow/
4. Host the binary on an HTTP server
5. Send a downlink command with payload "03" from ChirpStack
6. Monitor serial output to verify OTA progress

## Verifying LoRaWAN Communication

1. Monitor the device in ChirpStack - you should see uplink messages every TX_INTERVAL (default 60 seconds)
2. The decoded messages will show current readings in both milliamps and amps
3. Test downlink functionality by sending commands:
   - `{"led_command": "on"}` - Turn on the on-board LED
   - `{"led_command": "off"}` - Turn off the on-board LED
   - `{"led_command": "blink"}` - Make the on-board LED blink
   - `{"led_command": "ota"}` - Trigger OTA update

## Troubleshooting

### Common Issues

| Issue | Symptoms | Troubleshooting |
|-------|----------|----------------|
| Module initialization failure | "Failed to communicate with module" message | Check RAK3172 connections and power |
| Network join failure | "Failed to join LoRaWAN network" message | Verify credentials and gateway coverage |
| UART communication issues | Timeout messages for AT commands | Check UART connections and baud rate |
| WiFi connection failure | "Failed to connect to WiFi" message | Check SSID/password and signal strength |
| MQTT connection failure | "MQTT connection failed" message | Verify broker address and credentials |
| OTA update failure | "OTA failed" message | Check WiFi connection and server URL |
| Serial output issues | No serial output | Check USB connection and baud rate |

### Serial Output

Connect to the Pico's USB serial port (115200 baud) to view debug messages:

1. Messages are categorized by log level:
   - ERROR: Critical issues requiring attention
   - INFO: Normal operational information
   - DEBUG: Detailed information for debugging
   - VERBOSE: Very detailed tracing information

2. Configure the log level in config.h to control verbosity

## Power Considerations

The system can be powered in several ways:

1. **USB Power**: Connect the Pico W to a USB power source (5V)
2. **Battery Power**: Connect a LiPo battery to the VSYS and GND pins (3.7-5.5V)
3. **External Power**: Supply 3.3V directly to the 3V3 and GND pins

> **Note**: When using WiFi or BLE, power consumption increases significantly. Consider power requirements carefully for battery-powered applications.

## Customization

### Adjusting Measurement Intervals

To change how often measurements are taken and transmitted:

1. In config.h, modify the interval settings:
   ```cpp
   #define TX_INTERVAL 60000         // LoRaWAN interval (60 seconds)
   #define WIFI_TX_INTERVAL 10000    // MQTT interval (10 seconds)
   #define BLE_TX_INTERVAL 5000      // BLE update interval (5 seconds)
   ```

### Calibrating the Current Sensor

If the current readings seem inaccurate:

1. Adjust the WCS6800 sensitivity and offset voltage constants in config.h:
   ```cpp
   #define WCS6800_SENSITIVITY 0.0429 // V/A (may need calibration)
   #define WCS6800_OFFSET_VOLTAGE 1.65 // Volts (may vary by sensor)
   ```
2. Measure the actual output voltage at 0A current and adjust the offset
3. Apply a known current and adjust the sensitivity for accurate readings

## Advanced Configuration

### Adding New Features

The modular code structure makes it easy to add new features:

1. Create a new module header (.h) and implementation (.cpp) files
2. Add configuration options to config.h
3. Include and initialize the module in main.cpp
4. Integrate with the main processing loop

### Creating a Web Dashboard

You can create a web dashboard to display data by:

1. Setting up a web server that subscribes to the MQTT topic
2. Creating visualization using tools like Node-RED, Grafana, or custom web apps
3. Storing data in a database for historical analysis

### Expanding BLE Functionality

You can expand BLE functionality by adding more services and characteristics:

1. Define additional UUIDs in ble_manager.h
2. Create new characteristics in setupBLE()
3. Add handlers for notifications and commands

### Setting Up a Development Environment

For development with debugging:

1. Configure OpenOCD for Pico W debugging
2. Set up a debug configuration in PlatformIO
3. Use breakpoints and watch variables for debugging 