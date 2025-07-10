Requirements Document for Enhanced RPi Pico W Current Monitoring System
1. Introduction
This document outlines the requirements for enhancing an Arduino-based project running on a Raspberry Pi Pico W with a WCS6800 current sensor and RAK3172 LoRaWAN module. The Pico W's built-in WiFi and Bluetooth capabilities will be leveraged for Over-the-Air (OTA) updates and local data monitoring. The existing LoRaWAN downlink functionality (e.g., LED control commands, OTA trigger) is sufficient and will not be modified, with potential enhancements considered in the future. The enhancements aim to improve maintenance, debugging, and functionality by adding OTA updates via WiFi or Bluetooth, WiFi/Bluetooth data monitoring, a modular code structure, and preparation for future dashboard integration.
2. Objectives

Implement OTA updates using the Pico W's WiFi or Bluetooth, triggered by the existing LoRaWAN downlink command (e.g., payload 0x03).
Enable local data monitoring via WiFi (e.g., MQTT) and Bluetooth (e.g., serial terminal).
Refactor code to separate maintenance and debugging utilities into a dedicated file for improved maintainability.
Prepare the system for future web dashboard integration by formatting data appropriately.
Ensure compatibility with the existing hardware (Pico W, WCS6800, RAK3172) and optimize for low memory usage.

3. Scope

In-Scope:
OTA updates via Pico W's WiFi or Bluetooth, triggered by the existing LoRaWAN downlink.
WiFi-based data transmission to a local server or MQTT broker.
Bluetooth-based data monitoring via a smartphone app or terminal.
Creation of utils.h/utils.cpp for maintenance and debugging functions.
JSON data formatting for future dashboard integration.
Documentation and error handling for all new features.


Out-of-Scope:
Modifications to the existing LoRaWAN downlink functionality (e.g., new commands beyond 0x03 for OTA).
Development of the actual web dashboard (only data preparation included).
Support for additional microcontrollers beyond the Pico W.
Hardware modifications beyond the existing setup (Pico W, WCS6800, RAK3172).



4. Functional Requirements
4.1 Over-the-Air (OTA) Updates

Objective: Enable remote firmware updates via WiFi or Bluetooth to reduce physical access needs, using the existing LoRaWAN downlink trigger.
Requirements:
Use the Pico W's built-in WiFi (CYW43439) for OTA updates by downloading firmware from a user-specified HTTP server (URL stored in config.h).
Implement Bluetooth-based OTA updates using the Pico W's Bluetooth, allowing firmware transfer via a smartphone app (e.g., sending firmware chunks).
Use the existing LoRaWAN downlink command (payload 0x03) to trigger OTA updates, with the payload specifying the update method (WiFi or Bluetooth).
Implement security measures:
Checksum validation (e.g., CRC32) to verify firmware integrity.
Optional basic HTTP authentication for WiFi OTA (credentials in config.h).


Provide error handling for failed downloads or corrupted firmware, with rollback to the previous firmware version.
Log OTA process details (e.g., start, progress, success/failure) to Serial using a logging utility.
Ensure OTA updates do not disrupt current sensor readings or LoRaWAN communication.
Store OTA configuration (e.g., server URL, Bluetooth pairing details) in config.h.



4.2 WiFi Connectivity for Local Data Monitoring

Objective: Enable real-time data monitoring via the Pico W's WiFi.
Requirements:
Use the Pico W's WiFi to connect to a user-configured network (SSID and password stored in config.h).
Transmit current sensor data to a local server or MQTT broker (e.g., Mosquitto) in JSON format (e.g., {"current": value, "timestamp": millis()}).
Implement a configurable transmission interval (e.g., every 10 seconds, defined in config.h).
Include error handling for WiFi connection failures with automatic reconnection (e.g., 3 retries).
Ensure WiFi functionality is optional and can be disabled via config.h without affecting LoRaWAN or Bluetooth.
Log WiFi connection status, data transmission events, and errors to Serial using a logging utility.
Optimize WiFi operations for low power and memory usage.



4.3 Bluetooth Connectivity for Local Monitoring

Objective: Allow local data monitoring via the Pico W's Bluetooth.
Requirements:
Use the Pico W's built-in Bluetooth (CYW43439) to stream data to a smartphone or Bluetooth terminal.
Configure Bluetooth with a user-defined device name (stored in config.h).
Send current sensor data (e.g., current value, timestamp) in a human-readable format (e.g., "Current: X.XX A").
Support simple Bluetooth commands (e.g., request current reading, toggle LED).
Ensure Bluetooth functionality is optional and can be disabled via config.h without affecting LoRaWAN or WiFi.
Log Bluetooth connection status, data transmission events, and errors to Serial using a logging utility.
Handle Bluetooth connection drops with automatic reconnection attempts (e.g., 3 retries).



4.4 Modular Code Structure for Maintenance and Debugging

Objective: Improve code maintainability and debugging by isolating utility functions.
Requirements:
Create utils.h and utils.cpp files for maintenance and debugging utilities.
Include the following functions in utils.h/utils.cpp:
Logging Function: Log messages to Serial with configurable verbosity levels (e.g., INFO, DEBUG, ERROR) set in config.h.
Diagnostic Routine: Check WCS6800 sensor (ADC range), RAK3172 module (UART response), WiFi, and Bluetooth status.
Error Handling: Define error codes for common issues (e.g., LoRaWAN join failure, WiFi/Bluetooth connection loss) and implement recovery logic (e.g., retries).


Refactor existing code to use these utilities (e.g., replace Serial prints with logging function, use diagnostics for initialization).
Ensure main.cpp focuses on core functionality (sensor reading, LoRaWAN communication, OTA handling).
Add detailed comments in utils.h/utils.cpp explaining each function’s purpose and usage.
Optimize utilities for low memory usage on the Pico W (264KB SRAM, 2MB flash).



4.5 Preparation for Future Dashboard Integration

Objective: Format data for seamless integration with a future web dashboard.
Requirements:
Implement a function in utils.h/utils.cpp to format current sensor data as JSON (e.g., {"current": value, "timestamp": millis()}).
Allow JSON data to be sent via WiFi (preferred) or LoRaWAN (if payload size permits, e.g., <51 bytes for IN865 region).
Include placeholders or comments for future dashboard features (e.g., WebSocket or MQTT endpoints).
Ensure JSON formatting is lightweight to minimize memory usage.
Log JSON data to Serial for debugging during development.
Provide a configuration option in config.h to enable/disable JSON formatting.



5. Non-Functional Requirements

Performance:
Optimize code for low memory usage (Pico W: 264KB SRAM, 2MB flash).
Ensure OTA updates, WiFi, and Bluetooth operations do not delay sensor readings by more than 100ms.


Compatibility:
Maintain compatibility with existing hardware: WCS6800 on GP26 (ADC0), RAK3172 on GP0/TX, GP1/RX (UART0), LED on GP25, RST on GP2.
Use Arduino framework libraries compatible with Pico W (e.g., WiFi for CYW43439, BluetoothLowEnergy, PubSubClient for MQTT).


Reliability:
Implement error handling for all new features (e.g., WiFi/Bluetooth connection failures, OTA errors).
Include retry mechanisms for network operations (e.g., 3 retries for WiFi, Bluetooth, or LoRaWAN).


Maintainability:
Use consistent naming conventions (e.g., camelCase for functions) and detailed comments.
Suggest a .cursorrules file for Cursor AI to enforce coding standards (e.g., error handling patterns, consistent formatting).


Debugging:
Provide verbose logging for all features, configurable via config.h.
Include diagnostic functions to verify hardware and network status.



6. Hardware Requirements

Existing Hardware:
Raspberry Pi Pico W (RP2040 with CYW43439 for WiFi/Bluetooth).
WCS6800 current sensor (connected to GP26/ADC0, 3.3V, GND).
RAK3172 LoRaWAN module (connected to GP0/TX, GP1/RX, UART0).
LED on GP25, reset pin on GP2.


Additional Hardware:
No additional modules required, as Pico W provides WiFi and Bluetooth.
Ensure proper power supply (3.3V or 5V) for WCS6800 and RAK3172.



7. Software Requirements

Development Environment: PlatformIO with Arduino framework, supporting Pico W.
Libraries:
Use Pico W-compatible libraries: WiFi (for CYW43439 WiFi), BluetoothLowEnergy or BLE (for Bluetooth), PubSubClient (for MQTT).
Ensure libraries are lightweight to minimize memory usage.


Configuration:
Create a config.h file for user-configurable settings:
WiFi SSID and password.
Bluetooth device name.
OTA server URL and HTTP authentication credentials.
Debug verbosity level.
JSON formatting enable/disable.
Transmission intervals for WiFi and Bluetooth.


Ensure sensitive data (e.g., WiFi credentials) can be updated without modifying core code.



8. Implementation Steps

Refactor Existing Code:
Move debugging and utility functions (e.g., clearUartBuffer, sendATCommand) to utils.h/utils.cpp.
Replace Serial prints with a logging function from utils.h.
Add diagnostic functions for WCS6800, RAK3172, WiFi, and Bluetooth.


Implement OTA Updates:
Configure Pico W’s WiFi for HTTP-based firmware downloads.
Implement Bluetooth OTA for firmware transfer via a smartphone app.
Use existing listenForDownlink to handle OTA trigger command (0x03) with method selection (WiFi/Bluetooth).
Test OTA process with a sample firmware update.


Add WiFi Connectivity:
Configure Pico W’s WiFi to connect to a user-specified network.
Implement MQTT publishing for current sensor data.
Test data transmission to a local MQTT broker.


Add Bluetooth Connectivity:
Configure Pico W’s Bluetooth for data streaming and command handling.
Test with a Bluetooth terminal app on a smartphone.


Prepare for Dashboard:
Add JSON formatting function in utils.h/utils.cpp.
Modify sendLoRaWANPayload to optionally include JSON data.
Add comments for future WebSocket/MQTT integration.


Test and Debug:
Use diagnostic functions to verify hardware and network status.
Test OTA, WiFi, Bluetooth, and JSON formatting in isolation and together.
Monitor memory usage and optimize as needed.


Document and Finalize:
Add detailed comments for all new code.
Create a .cursorrules file for Cursor AI to enforce coding standards.
Provide setup instructions for libraries and configuration.



9. Deliverables

Updated main.cpp with core functionality and integration of new features.
New utils.h and utils.cpp files for maintenance and debugging utilities.
New config.h file for user-configurable settings.
Sample .cursorrules file for coding standards.
Documentation (in code comments and a README) explaining setup, usage, and debugging.
Test results verifying OTA, WiFi, Bluetooth, and JSON formatting functionality.

10. Assumptions and Constraints

Assumptions:
The Pico W’s WiFi and Bluetooth capabilities are sufficient for OTA and data monitoring.
A local MQTT broker or HTTP server is available for testing WiFi and OTA features.
The existing LoRaWAN downlink command (0x03) is sufficient for OTA triggers.
The user has access to PlatformIO and Pico W-compatible libraries.


Constraints:
Limited memory (264KB SRAM, 2MB flash) on the Pico W.
LoRaWAN payload size limits (e.g., 51 bytes for IN865 region) may restrict JSON data.
UART0 is used by RAK3172, so Bluetooth/WiFi may require software serial or careful pin management if additional UART is needed.



11. Success Criteria

OTA updates successfully apply firmware via WiFi or Bluetooth, triggered by the existing LoRaWAN downlink command.
WiFi and Bluetooth transmit current sensor data without interfering with LoRaWAN.
Maintenance and debugging utilities in utils.h/utils.cpp reduce debugging time and improve code readability.
JSON-formatted data is correctly generated and transmitted, ready for future dashboard integration.
All features operate within the Pico W’s memory and performance constraints.
Comprehensive logs and diagnostics enable quick issue resolution.

12. Risks and Mitigation

Risk: OTA updates fail due to network issues or corrupted firmware.
Mitigation: Implement checksum validation and rollback mechanism.


Risk: WiFi/Bluetooth operations consume excessive memory or power.
Mitigation: Use lightweight libraries and disable modules when not in use.


Risk: Complex code increases debugging difficulty.
Mitigation: Use modular structure, verbose logging, and diagnostic routines.


Risk: LoRaWAN payload size limits JSON data transmission.
Mitigation: Prioritize WiFi for JSON data; use compact binary format for LoRaWAN.



13. Future Considerations

Enhance LoRaWAN downlink functionality to support additional commands (e.g., configuration updates).
Develop a web dashboard to visualize JSON data sent via WiFi or MQTT.
Add support for additional sensors (e.g., voltage, temperature) to enhance monitoring.
Implement advanced OTA features (e.g., version checking, staged rollouts).
Explore low-power modes for WiFi/Bluetooth to reduce energy consumption.

14. Approval
This requirements document will be reviewed by the project owner to ensure alignment with goals. Any changes or clarifications will be incorporated before implementation begins.