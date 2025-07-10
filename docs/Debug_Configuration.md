# Debug Configuration Guide

## Overview

The WCS6800 LoRa Current Sensor system includes debug messages that provide detailed logging information during operation. This helps with troubleshooting, development, and understanding the system's behavior.

## Monitoring Debug Output

The Arduino implementation sends debug information through the Serial interface, which can be monitored using the Arduino IDE's Serial Monitor.

### How to Access Debug Output

1. Connect the Raspberry Pi Pico to your computer via USB
2. Open the Arduino IDE
3. Open the Serial Monitor (Tools > Serial Monitor)
4. Set the baud rate to 115200
5. Observe the debug messages in real-time

## Debug Output Categories

The system outputs the following information categories:

### 1. Hardware Readings
- Current values from the sensor
- Derived voltage readings

### 2. UART Communication
- AT commands sent to the RAK3172
- Raw responses from the RAK3172
- Command timeout information

### 3. Module Initialization
- Module readiness checks
- Individual parameter setting status
- Network join status

### 4. Data Processing
- Payload formatting details
- Value range checks and adjustments
- Data transmission confirmations

### 5. System Operation
- System initialization information
- Wait period notifications
- Error conditions

## Sample Debug Output

With the system running, you'll see output similar to:

```
==========================================================
WCS6800 Current Sensor LoRaWAN Monitor
==========================================================
UART: TX=GP0, RX=GP1 @ 115200 baud
ADC: WCS6800 on GP26
Device Address: 01d3257c
==========================================================
Initializing LoRaWAN ABP mode...
Waiting for RAK3172 module to be ready...
Attempt 1 to communicate with module...
Sending command: AT
AT
OK

Module is ready!
Setting ABP mode...
Sending command: AT+NJM=0
AT+NJM=0
OK

Setting Class C...
Sending command: AT+CLASS=C
AT+CLASS=C
OK

Setting IN865 region...
Sending command: AT+BAND=3
AT+BAND=3
OK

Setting device address...
Sending command: AT+DEVADDR=01d3257c
AT+DEVADDR=01d3257c
OK

Setting app session key...
Sending command: AT+APPSKEY=ef54ccd9b3d974e8736c60d916ad6e96
AT+APPSKEY=ef54ccd9b3d974e8736c60d916ad6e96
OK

Setting network session key...
Sending command: AT+NWKSKEY=06ebd62a3b4e2ed8d45d38d0f515988e
AT+NWKSKEY=06ebd62a3b4e2ed8d45d38d0f515988e
OK

Joining LoRaWAN network...
Sending command: AT+JOIN
AT+JOIN
OK

LoRaWAN initialized and joined successfully!
Starting main monitoring loop...

Current Reading: 0.125 A
Sending payload: 007D (Current: 0.125A)
Sending command: AT+SEND=2:007D
AT+SEND=2:007D
OK

Payload sent successfully
Listening for downlink messages...
No downlink received within timeout period
Waiting 60 seconds before next transmission...
```

## Error Messages

The system will output clear error messages when issues occur:

```
ERROR: Failed to communicate with module after multiple attempts
ERROR: Failed to set ABP mode
ERROR: Failed to join LoRaWAN network
ERROR: Failed to send payload
Timeout waiting for 'OK' response
```

## Customizing Debug Output

To add additional debug information:

1. Insert additional `Serial.print()` or `Serial.println()` statements in the code
2. Focus on critical areas such as:
   - Sensor reading calculations
   - Data encoding processes
   - Command response parsing

For example:
```cpp
// Add detailed sensor reading debug
float readCurrent() {
  int adcValue = analogRead(ADC_PIN);
  Serial.print("Raw ADC: ");
  Serial.println(adcValue);
  
  float voltage = (float(adcValue) / ADC_MAX_VALUE) * ADC_REF_VOLTAGE;
  Serial.print("Voltage: ");
  Serial.print(voltage, 3);
  Serial.println("V");
  
  float current = (voltage - WCS6800_OFFSET_VOLTAGE) / WCS6800_SENSITIVITY;
  return current;
}
```

## Using Serial Output for Development

When developing or modifying the system:

1. **Initial Setup**: Monitor all communications to verify proper operation
2. **Troubleshooting**: Look for error messages and unexpected values
3. **Normal Operation**: Focus on current readings and transmission status
4. **Integration Testing**: Verify downlink command reception and processing

## Impact on System Performance

With extensive serial output:
- Increased program size due to string constants
- Slightly higher power consumption due to serial communication
- Potential timing impacts due to longer processing for print statements

For production deployment with battery power, consider limiting serial output to essential information only. 