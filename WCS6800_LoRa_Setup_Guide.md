# WCS6800 LoRa Current Sensor - Setup Guide

## Prerequisites

Before setting up the system, ensure you have the following:

1. Raspberry Pi Pico with Arduino support installed
2. WCS6800 Current Sensor
3. RAK3172 LoRaWAN Module
4. LED (optional for visual feedback)
5. Jumper wires and breadboard
6. Micro USB cable for programming and power
7. Access to a ChirpStack LoRaWAN server and gateway
8. Arduino IDE with Raspberry Pi Pico board support

## Hardware Assembly

### Step 1: Connect the WCS6800 Current Sensor

1. Connect WCS6800 VCC to Raspberry Pi Pico 3.3V
2. Connect WCS6800 GND to Raspberry Pi Pico GND
3. Connect WCS6800 VOUT to Raspberry Pi Pico GPIO 26 (ADC0)

### Step 2: Connect the RAK3172 LoRaWAN Module

1. Connect RAK3172 VCC to Raspberry Pi Pico 3.3V
2. Connect RAK3172 GND to Raspberry Pi Pico GND
3. Connect RAK3172 TX to Raspberry Pi Pico GPIO 1 (UART0 RX)
4. Connect RAK3172 RX to Raspberry Pi Pico GPIO 0 (UART0 TX)
5. Connect RAK3172 RESET to Raspberry Pi Pico GPIO 30

### Step 3: Connect LED Indicator (Optional)

The on-board LED (GPIO 25) is used for visual feedback from downlink commands. No additional connection is required for this LED.

## Software Setup

### Step 1: Set Up Arduino IDE

1. Install the Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software)
2. Add Raspberry Pi Pico board support to Arduino IDE:
   - Open Arduino IDE
   - Go to File > Preferences
   - Add this URL to "Additional Boards Manager URLs": `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
   - Go to Tools > Board > Boards Manager
   - Search for "Raspberry Pi Pico" and install the board package

### Step 2: Set Up ChirpStack Server

1. Register a new device in ChirpStack with ABP activation
2. Note down the following credentials:
   - Device Address
   - Network Session Key
   - Application Session Key
3. Ensure the codec functions are applied to your application in ChirpStack:
   - Upload the `Chirpstack_codec.js` file to your ChirpStack application

### Step 3: Configure the Arduino Code

1. Open the `Pico_WCS6800.ino` file in Arduino IDE
2. Update the LoRaWAN ABP configuration with your device credentials:
   ```cpp
   const char* DevAdd = "01d3257c";  // Replace with your Device Address
   const char* NetKey = "06ebd62a3b4e2ed8d45d38d0f515988e";  // Replace with your Network Session Key
   const char* SessKey = "ef54ccd9b3d974e8736c60d916ad6e96";  // Replace with your Application Session Key
   ```
3. If needed, adjust the region setting in the `initializeLoRaWAN()` function:
   ```cpp
   {"AT+BAND=3", "IN865 region"},  // Change to your region's code
   ```
   Region codes:
   - EU868: 0
   - US915: 1
   - AU915: 2
   - IN865: 3
   - KR920: 4
   - AS923: 5
   - AS923-2: 6
   - AS923-3: 7
   - AS923-4: 8

### Step 4: Upload Code to Raspberry Pi Pico

1. Connect the Raspberry Pi Pico to your computer via USB
2. Press and hold the BOOTSEL button while connecting to enter programming mode
3. In Arduino IDE:
   - Select the appropriate board (Raspberry Pi Pico)
   - Select the correct port
   - Click the Upload button
4. Once uploaded, the Pico will restart and run the program automatically

## Testing the System

### Verifying Hardware Connections

When the system powers up, you should observe:

1. Serial monitor output showing initialization
2. Connection attempts to the RAK3172 module
3. Successful LoRaWAN network join messages

To view debug information:
1. Open the Serial Monitor in Arduino IDE
2. Set the baud rate to 115200
3. Monitor the initialization and operational messages

### Verifying LoRaWAN Communication

1. Monitor the device in ChirpStack - you should see uplink messages every 60 seconds
2. The decoded messages will show current readings in both milliamps and amps
3. Test downlink functionality by sending commands:
   - `{"led_command": "on"}` - Turn on the on-board LED
   - `{"led_command": "off"}` - Turn off the on-board LED
   - `{"led_command": "blink"}` - Make the on-board LED blink

## Troubleshooting

### Common Issues

| Issue | Symptoms | Troubleshooting |
|-------|----------|----------------|
| Module initialization failure | "Failed to communicate with module" message | Check RAK3172 connections and power |
| Network join failure | "Failed to join LoRaWAN network" message | Verify credentials and gateway coverage |
| UART communication issues | Timeout messages for AT commands | Check UART connections and baud rate |
| Main loop error | Error messages during operation | Check serial output for specific error details |
| System crash | No serial output | Check power supply and connections |

### Serial Output

Connect to the Pico's USB serial port (115200 baud) to view debug messages:

1. Messages beginning with "Sending command:" show AT commands sent to RAK3172
2. Response data from RAK3172 is displayed directly
3. Current readings are displayed every measurement cycle
4. Error messages will be prefixed with "ERROR:"

## Power Considerations

The system can be powered in several ways:

1. **USB Power**: Connect the Pico to a USB power source (5V)
2. **Battery Power**: Connect a LiPo battery to the VSYS and GND pins (3.7-5.5V)
3. **External Power**: Supply 3.3V directly to the 3V3 and GND pins

> **Note**: When using battery power, ensure proper voltage regulation and monitor battery levels.

## Customization

### Adjusting Measurement Interval

To change how often measurements are taken and transmitted:

1. Locate the delay at the end of the `loop()` function
2. Modify the delay time:
   ```cpp
   // For a 30-second interval instead of 60 seconds
   delay(30000);
   ```

### Calibrating the Current Sensor

If the current readings seem inaccurate:

1. Adjust the WCS6800 sensitivity and offset voltage constants:
   ```cpp
   #define WCS6800_SENSITIVITY 0.0429 // V/A (may need calibration)
   #define WCS6800_OFFSET_VOLTAGE 1.65 // Volts (may vary by sensor)
   ```
2. Measure the actual output voltage at 0A current and adjust the offset
3. Apply a known current and adjust the sensitivity for accurate readings

## Advanced Configuration

### Changing to OTAA Mode

To use Over-The-Air Activation instead of ABP:

1. Obtain DevEUI, AppEUI, and AppKey from ChirpStack
2. Modify the `initializeLoRaWAN()` function to use OTAA parameters
3. Update the ChirpStack codec as needed for OTAA operation

### Adding Additional Sensors

The system can be expanded with additional sensors:

1. Connect new sensors to available GPIO pins
2. Initialize and read them in the code
3. Expand the payload format to include additional sensor data
4. Update the ChirpStack codec to decode the expanded payload 