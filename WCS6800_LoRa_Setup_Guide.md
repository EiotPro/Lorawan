# WCS6800 LoRa Current Sensor - Setup Guide

## Prerequisites

Before setting up the system, ensure you have the following:

1. Raspberry Pi Pico with MicroPython installed
2. WCS6800 Current Sensor
3. RAK3172 LoRaWAN Module
4. Three LEDs (Red, Green, Blue) with appropriate current-limiting resistors
5. Jumper wires and breadboard
6. Micro USB cable for programming and power
7. Access to a ChirpStack LoRaWAN server and gateway

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
5. Connect RAK3172 RESET to Raspberry Pi Pico GPIO 2

### Step 3: Connect LED Indicators

1. **Red LED**:
   - Connect anode through a 220Ω resistor to GPIO 5
   - Connect cathode to GND

2. **Blue LED**:
   - Connect anode through a 220Ω resistor to GPIO 0
   - Connect cathode to GND

3. **Green LED**:
   - Connect anode through a 220Ω resistor to GPIO 3
   - Connect cathode to GND

> **Note**: The blue LED shares GPIO 0 with UART0 TX, so it will illuminate during UART transmission. This serves as a visual indicator for communication with the RAK3172 module.

## Software Setup

### Step 1: Set Up ChirpStack Server

1. Register a new device in ChirpStack with ABP activation
2. Note down the following credentials:
   - Device Address
   - Network Session Key
   - Application Session Key
3. Ensure the codec functions are applied to your application in ChirpStack:
   - Upload the `Chirpstack_codec.js` file to your ChirpStack application

### Step 2: Configure the Pico Code

1. Open the `Pico_Wcs6800.py` file
2. Update the LoRaWAN ABP configuration with your device credentials:
   ```python
   DevAdd = "01d3257c"  # Replace with your Device Address
   NetKey = "06ebd62a3b4e2ed8d45d38d0f515988e"  # Replace with your Network Session Key
   SessKey = "ef54ccd9b3d974e8736c60d916ad6e96"  # Replace with your Application Session Key
   ```
3. If needed, adjust the region setting in the `initialize_lorawan()` function:
   ```python
   ("AT+BAND=3", "IN865 region"),  # Change to your region's code
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

### Step 3: Upload Code to Raspberry Pi Pico

1. Connect the Raspberry Pi Pico to your computer via USB
2. Press and hold the BOOTSEL button while connecting to enter programming mode
3. Copy the `Pico_Wcs6800.py` file to the Pico (rename to `main.py` if you want it to run automatically)
4. Safely eject the Pico and reset it

## Testing the System

### Verifying Hardware Connections

When the system powers up, you should observe:

1. All LEDs will briefly flash once in sequence
2. The red LED will illuminate while waiting for the RAK3172 module
3. The green LED will illuminate during LoRaWAN initialization
4. If initialization is successful, the green LED will remain on

### Verifying LoRaWAN Communication

1. Monitor the device in ChirpStack - you should see uplink messages every 60 seconds
2. The decoded messages will show current readings in both milliamps and amps
3. Test downlink functionality by sending commands:
   - `{"led_command": "on"}` - Turn on the on-board LED
   - `{"led_command": "off"}` - Turn off the on-board LED
   - `{"led_command": "blink"}` - Make the on-board LED blink

## Troubleshooting

### LED Status Indicators

Use the LED indicators to diagnose common issues:

| Issue | LED Behavior | Troubleshooting |
|-------|-------------|----------------|
| Module initialization failure | Red LED flashing rapidly at startup | Check RAK3172 connections and power |
| Network join failure | No green LED after initialization | Verify credentials and gateway coverage |
| UART communication issues | Blue LED stays on for extended periods | Check UART connections and baud rate |
| Main loop error | Red LED flashing during operation | Check serial output for error messages |
| System crash | No LED activity | Check power supply and connections |

### Serial Output

Connect to the Pico's USB serial port (typically 115200 baud) to view debug messages:

1. Messages beginning with "Sending command:" show AT commands sent to RAK3172
2. Messages beginning with "Response:" show RAK3172 replies
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

1. Locate the main loop wait period at the end of the `main()` function
2. Modify the sleep time and LED pulse count:
   ```python
   # For a 30-second interval instead of 60 seconds
   print(f"Waiting 30 seconds before next transmission...")
   for _ in range(3):  # 3 pulses of 10 seconds each
       green_led.value(1)
       time.sleep(1)
       green_led.value(0)
       time.sleep(9)
   ```

### Calibrating the Current Sensor

If the current readings seem inaccurate:

1. Adjust the WCS6800 sensitivity and offset voltage constants:
   ```python
   WCS6800_SENSITIVITY = 0.0429  # V/A (may need calibration)
   WCS6800_OFFSET_VOLTAGE = 1.65  # Volts (may vary by sensor)
   ```
2. Measure the actual output voltage at 0A current and adjust the offset
3. Apply a known current and adjust the sensitivity for accurate readings

## Advanced Configuration

### Changing to OTAA Mode

To use Over-The-Air Activation instead of ABP:

1. Obtain DevEUI, AppEUI, and AppKey from ChirpStack
2. Modify the `initialize_lorawan()` function to use OTAA parameters
3. Update the ChirpStack codec as needed for OTAA operation

### Adding Additional Sensors

The system can be expanded with additional sensors:

1. Connect new sensors to available GPIO pins
2. Initialize and read them in the code
3. Expand the payload format to include additional sensor data
4. Update the ChirpStack codec to decode the expanded payload 