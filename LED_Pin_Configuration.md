# LED Pin Configuration

## Pin Assignments

| GPIO Pin | Function             | LED Color | Purpose                           |
|----------|----------------------|-----------|-----------------------------------|
| GPIO 0   | UART0 TX             | Blue      | Indicates data transmission       |
| GPIO 1   | UART0 RX             | -         | Data reception from RAK3172       |
| GPIO 2   | RAK3172 Reset        | -         | Hardware reset for RAK3172 module |
| GPIO 3   | Status Indicator     | Green     | System status                     |
| GPIO 5   | UART1 RX             | Red       | Module status and downlink        |
| GPIO 25  | On-board LED         | On-board  | Downlink command indicator        |
| GPIO 26  | ADC0 (Analog Input)  | -         | WCS6800 current sensor input      |

## LED Function Description

### Blue LED (GPIO 0)
The blue LED is connected to the UART0 TX pin and serves as a visual indicator when data is being transmitted to the RAK3172 LoRa module. This allows users to observe communication activity between the Pico and the RAK3172 module.

**Key Functions**:
- Lights up during AT command transmission
- Lights up during payload transmission
- Automatically turns off when transmission is complete or times out

### Red LED (GPIO 5)
The red LED is connected to the UART1 RX pin and serves multiple roles in indicating the system status, particularly related to the RAK3172 module readiness and downlink reception.

**Key Functions**:
- Lights up when waiting for the RAK3172 module to be ready
- Lights up when listening for downlink messages
- Flashes rapidly to indicate errors or failures

### Green LED (GPIO 3)
The green LED serves as the primary system status indicator, showing the overall state of the LoRaWAN connection and operation.

**Key Functions**:
- Lights up during LoRaWAN initialization
- Stays on when successfully connected to LoRaWAN network
- Pulses during the waiting period between transmissions
- Turns off when LoRaWAN connection fails

### On-board LED (GPIO 25)
The Raspberry Pi Pico's on-board LED is used as a remotely controllable indicator that responds to downlink commands from the LoRaWAN server.

**Key Functions**:
- Can be turned on via downlink command 0x01
- Can be turned off via downlink command 0x02
- Can be made to blink via downlink command 0x04

## Implementation Notes

The LED controls are implemented in software as standard GPIO outputs:

```python
red_led = machine.Pin(5, machine.Pin.OUT)
blue_led = machine.Pin(0, machine.Pin.OUT)
green_led = machine.Pin(3, machine.Pin.OUT)
led = machine.Pin(25, machine.Pin.OUT)  # On-board LED
```

All LEDs are initialized in the OFF state at startup, and a brief LED test sequence is performed during initialization to verify proper functioning.

## Electrical Considerations

- All LEDs should be connected with appropriate current-limiting resistors (typically 220-330 ohms)
- The GPIO pins on the Raspberry Pi Pico can source/sink a maximum of 12mA per pin
- For higher brightness or reliability in industrial environments, consider using transistor drivers for the LEDs
- Keep in mind that GPIO 0 is also used for UART0 TX, so the blue LED will flicker during UART transmission

## Troubleshooting Using LEDs

The LED indicators can be used to diagnose common issues:

1. **Red LED flashing rapidly at startup**: RAK3172 module initialization failed
2. **No green LED after initialization**: Failed to join LoRaWAN network
3. **Blue LED stays on for extended periods**: UART communication issues or timeouts
4. **Red LED flashing during normal operation**: Error in the main program loop
5. **No LED activity during expected transmission**: Check power or software crash

## Custom LED Patterns

The system can be extended to support additional LED patterns for different states or conditions by modifying the relevant functions in the code. For example:

- Different blink patterns to indicate signal strength
- Color combinations to show battery status (if powered by battery)
- Sequential patterns to indicate different operational modes 