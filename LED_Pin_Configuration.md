# LED Pin Configuration

## Pin Assignments

| GPIO Pin | Function             | LED Color | Purpose                           |
|----------|----------------------|-----------|-----------------------------------|
| GPIO 0   | UART0 TX             | -         | Data transmission to RAK3172      |
| GPIO 1   | UART0 RX             | -         | Data reception from RAK3172       |
| GPIO 25  | On-board LED         | On-board  | Downlink command indicator        |
| GPIO 26  | ADC0 (Analog Input)  | -         | WCS6800 current sensor input      |
| GPIO 30  | RAK3172 Reset        | -         | Hardware reset for RAK3172 module |

## LED Function Description

### On-board LED (GPIO 25)
The Raspberry Pi Pico's on-board LED is used as a remotely controllable indicator that responds to downlink commands from the LoRaWAN server.

**Key Functions**:
- Can be turned on via downlink command 0x01
- Can be turned off via downlink command 0x02
- Can be made to blink via downlink command 0x04

## Implementation Notes

The LED control is implemented in software as a standard GPIO output:

```cpp
#define LED_PIN 25
pinMode(LED_PIN, OUTPUT);
```

The LED is initialized in the OFF state at startup.

## Electrical Considerations

- The on-board LED is already connected with an appropriate current-limiting resistor
- The GPIO pins on the Raspberry Pi Pico can source/sink a maximum of 12mA per pin
- For higher brightness or reliability in industrial environments, consider adding external LEDs with transistor drivers

## Controlling the LED

The LED is controlled through downlink commands sent from the ChirpStack server:

```
// To turn the LED ON
{"led_command": "on"}

// To turn the LED OFF
{"led_command": "off"}

// To make the LED blink three times
{"led_command": "blink"}
```

## Customizing LED Behavior

The system can be extended to support additional LED patterns by modifying the `listenForDownlink()` function in the code:

```cpp
if (payload == "01") {
  Serial.println("LED ON command received");
  digitalWrite(LED_PIN, HIGH);
} else if (payload == "02") {
  Serial.println("LED OFF command received");
  digitalWrite(LED_PIN, LOW);
} else if (payload == "04") {
  Serial.println("LED BLINK command received");
  for (int i = 0; i < 3; i++) { // Blink 3 times
    digitalWrite(LED_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    delay(300);
  }
} else {
  Serial.print("Unknown command: ");
  Serial.println(payload);
}
```

You can add more complex patterns or additional commands as needed. 