# Testing Instructions for WCS6800 Current Sensor

## Initial Setup

1. Make sure your platformio.ini file is correctly configured using the Earlephilhower core:
   ```ini
   [env]
   platform = https://github.com/maxgerhardt/platform-raspberrypi.git
   framework = arduino
   board_build.core = earlephilhower
   monitor_speed = 115200

   [env:rpipicow]
   board = rpipicow
   ```

2. Check your config.h file and make sure features are disabled for initial testing:
   - Set `WIFI_ENABLED` to `false`
   - Set `BLE_ENABLED` to `false`
   - Set `OTA_ENABLED` to `false`

## Phased Testing

### Phase 1: Basic Functionality

1. Build and upload the project with just the core functionality:
   ```
   pio run -t upload -e rpipicow
   ```

2. Open the serial monitor to check for proper initialization:
   ```
   pio device monitor -b 115200
   ```

3. Verify that:
   - Serial output shows correct initialization
   - WCS6800 sensor readings appear
   - LoRaWAN initialization is successful
   - Data is transmitted over LoRaWAN

### Phase 2: Add WiFi (After Phase 1 is successful)

1. Update config.h:
   - Change `WIFI_ENABLED` to `true`
   - Update WiFi credentials:
     ```c
     const char* const WIFI_SSID = "YourActualSSID"; 
     const char* const WIFI_PASSWORD = "YourActualPassword";
     ```

2. Build and upload again:
   ```
   pio run -t upload -e rpipicow
   ```

3. Verify that:
   - WiFi connects successfully 
   - If MQTT is enabled, verify that data is published to MQTT broker

### Phase 3: Test OTA (After Phase 2 is successful)

1. Update config.h:
   - Change `OTA_ENABLED` to `true`
   - Configure OTA server details:
     ```c
     const char* const OTA_SERVER_URL = "http://your-local-server/firmware.bin";
     ```

2. Build and upload:
   ```
   pio run -t upload -e rpipicow
   ```

3. Test OTA functionality by sending LoRaWAN downlink command (payload "03")

## Troubleshooting

### Serial Output Issues
- Make sure monitor_speed is set to 115200
- Check if the device is properly connected

### LoRaWAN Connection Issues
- Verify RAK3172 module is correctly wired
- Check ABP credentials in config.h

### WiFi Connection Issues
- Verify SSID and password
- Check that Pico W is within range of your WiFi network 