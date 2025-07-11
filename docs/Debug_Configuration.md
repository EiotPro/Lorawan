# Debug Configuration Guide

## Overview

The WCS6800 LoRa Current Sensor system includes a comprehensive logging framework that provides structured debug information during operation. This helps with troubleshooting, development, and understanding the system's behavior across all communication channels (LoRaWAN, WiFi, and BLE).

## Log Level Configuration

The system implements five distinct log levels that can be configured in `config.h`:

```cpp
// --- Debug Configuration ---
#define DEBUG_LEVEL 2  // 0=OFF, 1=ERROR, 2=INFO, 3=DEBUG, 4=VERBOSE
```

Log levels provide granular control over the amount of detail in the debug output:

| Level | Macro         | Description | Usage |
|-------|--------------|-------------|-------|
| 0     | LOG_NONE     | No output   | Production deployment with minimal overhead |
| 1     | LOG_ERROR    | Errors only | Error conditions requiring attention |
| 2     | LOG_INFO     | Informational | Normal but significant operational events |
| 3     | LOG_DEBUG    | Detailed debugging | Detailed information for debugging |
| 4     | LOG_VERBOSE  | Verbose debugging | Maximum detail including raw data |

## Monitoring Debug Output

The system outputs log messages through the Serial interface, which can be monitored using the PlatformIO Serial Monitor or other serial terminal.

### How to Access Debug Output

1. Connect the Raspberry Pi Pico W to your computer via USB
2. Open a serial monitor:
   ```
   pio device monitor -b 115200
   ```
   or in VS Code, click on the "Serial Monitor" button in the PlatformIO toolbar
3. Observe the structured log messages in real-time

## Log Output Format

Each log message follows a consistent format:

```
[LEVEL] Message content
```

Where `LEVEL` is one of:
- `ERROR`: Critical issues requiring immediate attention
- `INFO`: Normal operational information
- `DEBUG`: Detailed information for debugging
- `VERBOSE`: Extremely detailed tracing information

## Debug Information Categories

The system outputs the following information categories:

### 1. System Status
- Initialization steps
- Hardware diagnostics results
- Feature enablement status
- Operational state transitions

### 2. LoRaWAN Communication
- AT commands sent to the RAK3172
- Raw responses from the RAK3172
- Network join status
- Data transmission confirmations
- Downlink command reception

### 3. WiFi & MQTT Operations
- WiFi connection attempts and status
- MQTT broker connection status
- Data publishing events
- MQTT message reception

### 4. BLE Operations
- BLE initialization status
- Device connections and disconnections
- Characteristic updates
- Command reception

### 5. Sensor Readings
- Raw ADC values
- Voltage conversions
- Current calculations
- JSON formatting results

### 6. OTA Updates
- Update triggers
- Download progress
- Verification status
- Application status

## Using the Logging Functions

The system provides several logging functions in `utils.cpp` that are used throughout the code:

```cpp
// Basic logging functions
void log_error(const char* msg);          // Error conditions
void log_info(const char* msg);           // Informational messages
void log_debug(const char* msg);          // Debug information
void log_verbose(const char* msg);        // Verbose debug information

// Formatted logging (printf-style)
void log_format(uint8_t level, const char* format, ...);
```

### Example Usage

```cpp
// Simple message logging
log_info("System initialized");
log_error("Failed to connect to WiFi");

// Formatted message logging
log_format(LOG_DEBUG, "Current reading: %.3f A", currentValue);
log_format(LOG_INFO, "Connected to MQTT broker at %s:%d", MQTT_BROKER, MQTT_PORT);
```

## Sample Debug Output

With `DEBUG_LEVEL` set to `2` (INFO), you'll see output similar to:

```
[INFO] ==========================================================
[INFO] WCS6800 Current Sensor LoRaWAN Monitor
[INFO] ==========================================================
[INFO] UART: TX=GP0, RX=GP1 @ 115200 baud
[INFO] ADC: WCS6800 on GP26
[INFO] Device Address: 01d3257c
[INFO] ==========================================================
[INFO] Running full diagnostics...
[INFO] Checking WCS6800 sensor...
[INFO] WCS6800 sensor check passed. Voltage: 1.65 V
[INFO] Checking RAK3172 module...
[DEBUG] Clearing UART buffer
[DEBUG] Sending command: AT
[DEBUG] Command succeeded
[INFO] RAK3172 module check passed
[INFO] All diagnostics passed
[INFO] Setting up WiFi...
[INFO] Connecting to WiFi network: MyNetwork
[INFO] WiFi connected with IP: 192.168.1.105
[INFO] WiFi setup complete
[INFO] Setting up MQTT client...
[INFO] MQTT setup complete
[INFO] WiFi initialized successfully
[INFO] BLE requested but not fully supported on Pico W with current setup
[INFO] BLE functionality will be simulated/disabled
[INFO] OTA update system initialized successfully
[INFO] Initializing LoRaWAN ABP mode...
[INFO] Waiting for RAK3172 module to be ready...
[INFO] Attempt 1 to communicate with module...
[DEBUG] Clearing UART buffer
[DEBUG] Sending command: AT
[DEBUG] Command succeeded
[INFO] Module is ready!
[INFO] Setting ABP mode...
[DEBUG] Sending command: AT+NJM=0
[DEBUG] Command succeeded
[INFO] Setting Class C...
[DEBUG] Sending command: AT+CLASS=C
[DEBUG] Command succeeded
[INFO] Setting IN865 region...
[DEBUG] Sending command: AT+BAND=3
[DEBUG] Command succeeded
[INFO] Setting device address...
[DEBUG] Sending command: AT+DEVADDR=01d3257c
[DEBUG] Command succeeded
[INFO] Setting app session key...
[DEBUG] Sending command: AT+APPSKEY=ef54ccd9b3d974e8736c60d916ad6e96
[DEBUG] Command succeeded
[INFO] Setting network session key...
[DEBUG] Sending command: AT+NWKSKEY=06ebd62a3b4e2ed8d45d38d0f515988e
[DEBUG] Command succeeded
[INFO] Joining LoRaWAN network...
[DEBUG] Sending command: AT+JOIN
[DEBUG] Command succeeded
[INFO] LoRaWAN initialized and joined successfully!
[INFO] Starting main monitoring loop...

[INFO] Current Reading: 0.125 A
[DEBUG] JSON: {"current":0.125,"timestamp":15023,"unit":"A"}
[INFO] Publishing to MQTT topic: wcs6800/data
[INFO] MQTT publish successful
[INFO] Sending payload: 007D (Current: 0.125 A)
[DEBUG] Sending command: AT+SEND=2:007D
[DEBUG] Command succeeded
[INFO] Payload sent successfully
[INFO] Listening for downlink messages...
[INFO] No downlink received within timeout period
```

## Customizing Debug Output

To add additional debug information or modify existing logging:

1. Select the appropriate log level for your message:
   - `LOG_ERROR`: Use for critical errors
   - `LOG_INFO`: Use for normal operational messages
   - `LOG_DEBUG`: Use for detailed debug information
   - `LOG_VERBOSE`: Use for very detailed tracing

2. Choose the appropriate logging function:
   - `log_error()`, `log_info()`, etc. for simple messages
   - `log_format()` for formatted messages with variables

3. Add the logging calls to your code at key points

Example:
```cpp
bool connectToWiFi() {
    log_info("Attempting to connect to WiFi");
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    uint8_t attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 20) {
        delay(500);
        log_verbose(".");
        attempt++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        log_format(LOG_INFO, "WiFi connected with IP: %s", WiFi.localIP().toString().c_str());
        return true;
    } else {
        log_error("Failed to connect to WiFi");
        return false;
    }
}
```

## Using Debug Output for Development

When developing or modifying the system:

1. **Initial Setup**: Set `DEBUG_LEVEL` to 4 (VERBOSE) to see all details
2. **Troubleshooting**: Look for ERROR messages and track the flow through DEBUG messages
3. **Normal Operation**: Set `DEBUG_LEVEL` to 2 (INFO) for operational status
4. **Production**: Set `DEBUG_LEVEL` to 1 (ERROR) or 0 (NONE) to minimize overhead

## Performance Considerations

The logging framework is designed to minimize impact when higher log levels are disabled:

- Disabled log levels (higher than `DEBUG_LEVEL`) have near-zero overhead
- String constants are only stored in flash if their log level is enabled
- Formatted logging with `log_format()` only processes arguments if the log level is active

For production deployment with battery power, consider setting `DEBUG_LEVEL` to 0 or 1 to minimize power consumption from serial communication. 