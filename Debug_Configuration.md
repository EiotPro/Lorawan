# Debug Configuration Guide

## Overview

The WCS6800 LoRa Current Sensor system includes a debug mode that provides detailed logging information during operation. This helps with troubleshooting, development, and understanding the system's behavior.

## Enabling/Disabling Debug Mode

Debug mode is controlled by a single boolean flag at the top of the `Pico_Wcs6800.py` file:

```python
# --- Debug Configuration ---
DEBUG = True  # Set to False to disable debug print statements
```

- When `DEBUG = True`: Detailed logging is displayed
- When `DEBUG = False`: Only essential information and errors are displayed

## Debug Output Categories

When debug mode is enabled, the following additional information is logged:

### 1. Hardware Readings
- ADC raw values
- Voltage readings
- Detailed current calculations

### 2. UART Communication
- Bytes cleared from UART buffer
- Full response data from RAK3172
- Command success confirmations

### 3. Module Initialization
- Individual module initialization attempts
- Parameter setting details

### 4. Data Processing
- Value range limits and adjustments
- Raw data reception details

### 5. System Operation
- Measurement cycle tracking
- Wait period progress
- LED test sequence execution

## Sample Debug Output

With debug mode enabled, you'll see output similar to:

```
===============================================================
WCS6800 Current Sensor LoRaWAN Monitor
===============================================================
Debug mode: Enabled
UART: TX=GP0, RX=GP1 @ 115200 baud
ADC: WCS6800 on GP26
Device Address: 01d3257c
LED Indicators: Red=GP5, Blue=GP0, Green=GP3
===============================================================
Running LED test sequence
Initializing LoRaWAN ABP mode...
Waiting for RAK3172 module to be ready...
Attempt 1 to communicate with module...
Sending command: AT
Response: AT
OK

Command successful, found 'OK'
Module is ready!
Setting ABP mode...
Sending command: AT+NJM=0
Response: AT+NJM=0
OK

Command successful, found 'OK'
[...additional initialization details...]
LoRaWAN initialized and joined successfully!
Starting main monitoring loop...

--- Measurement cycle #1 ---
ADC raw: 2048, Voltage: 1.650V, Current: 0.000A

Current Reading: 0.000 A
Sending payload: 0000 (Current: 0.000A)
[...transmission details...]
Payload sent successfully
Waiting for downlink... 5/15s
Waiting for downlink... 10/15s
No downlink received within timeout period
Waiting 60 seconds before next transmission...
Wait period 10/60 seconds
Wait period 20/60 seconds
[...and so on...]
```

## Regular (Non-Debug) Output

When debug mode is disabled, only essential information is displayed:

```
===============================================================
WCS6800 Current Sensor LoRaWAN Monitor
===============================================================
Debug mode: Disabled
UART: TX=GP0, RX=GP1 @ 115200 baud
ADC: WCS6800 on GP26
Device Address: 01d3257c
LED Indicators: Red=GP5, Blue=GP0, Green=GP3
===============================================================
Initializing LoRaWAN ABP mode...
Waiting for RAK3172 module to be ready...
Sending command: AT
Module is ready!
Joining LoRaWAN network...
LoRaWAN initialized and joined successfully!
Starting main monitoring loop...

Current Reading: 0.000 A
Sending payload: 0000 (Current: 0.000A)
Payload sent successfully
Waiting 60 seconds before next transmission...
```

## Always-Displayed Information

Regardless of debug mode, the following information is always displayed:

1. **Critical system status**:
   - Startup banner and configuration
   - LoRaWAN connection status
   - Main loop start

2. **Operational data**:
   - Current readings (in Amperes)
   - Payload transmission status
   - Wait period notifications

3. **Error conditions**:
   - UART errors
   - Module communication failures
   - LoRaWAN join failures
   - Main loop exceptions

## Using Debug Mode for Development

When developing or modifying the system:

1. **Initial Setup**: Start with `DEBUG = True` to see detailed operation
2. **Troubleshooting**: Keep debug enabled to diagnose problems
3. **Normal Operation**: Set to `DEBUG = False` for cleaner output
4. **Production Deployment**: Always set to `DEBUG = False` to minimize resource usage and improve performance

## Impact on System Performance

With debug mode enabled:
- Increased memory usage due to string formatting
- Slightly higher power consumption due to additional UART usage
- Potential timing impacts due to longer processing for print statements

## Additional Debug Tools

In addition to the debug print statements, the system provides visual debugging through the LED indicators:

- **Red LED**: Module communication status and errors
- **Blue LED**: Data transmission activity
- **Green LED**: System status and operational state
- **On-board LED**: Downlink command reception 