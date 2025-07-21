# LED Pin Configuration

## Pin Assignments

| GPIO Pin | Function             | LED Color | Purpose                           |
|----------|----------------------|-----------|-----------------------------------|
| GPIO 0   | UART0 TX             | -         | Data transmission to RAK3172      |
| GPIO 1   | UART0 RX             | -         | Data reception from RAK3172       |
| GPIO 25  | On-board LED         | On-board  | Visual indicator for commands     |
| GPIO 26  | ADC0 (Analog Input)  | -         | WCS6800 current sensor input      |
| GPIO 2   | RAK3172 Reset        | -         | Hardware reset for RAK3172 module |

## LED Function Description

### On-board LED (GPIO 25)
The Raspberry Pi Pico W's on-board LED is used as a multi-purpose indicator that can be controlled via multiple channels:
- LoRaWAN downlink commands from the server
- BLE commands from a smartphone or other BLE device
- System status indicators

**Control Methods**:
1. **LoRaWAN Downlink Control**:
   - Turn ON via downlink command 0x01
   - Turn OFF via downlink command 0x02
   - Blink pattern via downlink command 0x04
   - Trigger OTA update via downlink command 0x03 (LED will flash during update process)

2. **BLE Control** (when enabled):
   - Turn ON via "LED_ON" command
   - Turn OFF via "LED_OFF" command
   - Blink via "BLINK_LED" command

3. **System Status Indicators** (automatic):
   - WiFi connection in progress: rapid blinking
   - OTA update in progress: slow pulse pattern
   - Error condition: SOS pattern (three short, three long, three short)

## Implementation Notes

The LED control is implemented through a combination of direct GPIO control and the utils module:

```cpp
// Basic LED control
#define LED_PIN 25
pinMode(LED_PIN, OUTPUT);
digitalWrite(LED_PIN, HIGH);  // ON
digitalWrite(LED_PIN, LOW);   // OFF

// Blink pattern (implemented in utils.cpp)
void blinkLED(uint8_t count, uint16_t onTime, uint16_t offTime) {
  for (uint8_t i = 0; i < count; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(onTime);
    digitalWrite(LED_PIN, LOW);
    delay(offTime);
  }
}
```

## Electrical Considerations

- The on-board LED is already connected with an appropriate current-limiting resistor
- The GPIO pins on the Raspberry Pi Pico W can source/sink a maximum of 12mA per pin
- For higher brightness or reliability in industrial environments, consider adding external LEDs with transistor drivers

## Controlling the LED

### Via LoRaWAN

The LED is controlled through downlink commands sent from the ChirpStack server:

```
// To turn the LED ON
{"led_command": "on"}

// To turn the LED OFF
{"led_command": "off"}

// To make the LED blink three times
{"led_command": "blink"}

// To trigger OTA update (LED will indicate progress)
{"led_command": "ota"}
```

### Via BLE (when enabled)

Connect to the device using a BLE client app and write to the command characteristic:

1. Connect to "WCS6800_Monitor" device
2. Find the Command Service (UUID in BLE documentation)
3. Write to the Command Characteristic:
   - "LED_ON" - Turn on LED
   - "LED_OFF" - Turn off LED
   - "BLINK_LED" - Blink LED pattern

## LED Indicator Patterns

The LED is used to communicate different system states through specific blink patterns:

| Pattern | Description | Meaning |
|---------|-------------|---------|
| Solid ON | Continuous light | LED ON command received or process running |
| OFF | No light | LED OFF command received or process stopped |
| Triple blink | 3 short blinks | LED BLINK command received |
| Rapid blinking | Fast ON/OFF cycles | WiFi connection in progress |
| Slow pulse | Gradual ON/OFF | OTA update downloading |
| SOS pattern | 3 short, 3 long, 3 short | System error detected |
| Single short flash | Brief ON then OFF | Successful data transmission |

## Customizing LED Behavior

The system can be extended to support additional LED patterns by modifying the utils module and the corresponding handlers:

### Adding a New LoRaWAN Command Pattern:

```cpp
// In listenForDownlink() in main.cpp
else if (payload == "05") {  // New command code
  log_info("LED ALTERNATING PATTERN command received");
  alternatingPattern();  // Custom pattern function
}

// In utils.cpp
void alternatingPattern() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    delay(300);
  }
}
```

### Adding a New BLE Command Pattern:

```cpp
// In ble_manager.cpp
void processBLECommand(const char* command) {
  if (strcmp(command, "LED_ON") == 0) {
    digitalWrite(LED_PIN, HIGH);
  } else if (strcmp(command, "LED_OFF") == 0) {
    digitalWrite(LED_PIN, LOW);
  } else if (strcmp(command, "BLINK_LED") == 0) {
    blinkLED(3, 300, 300);
  } else if (strcmp(command, "ALTERNATE_LED") == 0) {
    alternatingPattern();  // Custom pattern function
  }
}
``` 