#Working code with chipstack Author Amir
import machine
import time
import ubinascii # For hex conversion

# --- Debug Configuration ---
DEBUG = True  # Set to False to disable debug print statements

def debug_print(*args, **kwargs):
    """Print only when DEBUG is True"""
    if DEBUG:
        print(*args, **kwargs)

# --- WCS6800 Sensor Configuration ---
# Connect WCS6800 VOUT to RPi Pico ADC pin (e.g., GP26)
# Connect WCS6800 VCC to RPi Pico 3.3V
# Connect WCS6800 GND to RPi Pico GND

# ADC pin for WCS6800 output (GP26 corresponds to ADC0)
# Refer to RPi Pico pinout for correct ADC mapping
ADC_PIN = 26 # GPIO26 / ADC0

# ADC object
adc = machine.ADC(machine.Pin(ADC_PIN))

# RPi Pico ADC has 12-bit resolution (0-4095)
ADC_MAX_VALUE = 4095.0

# RPi Pico ADC reference voltage is 3.3V
ADC_REF_VOLTAGE = 3.3 # Volts

# WCS6800 specifications (when powered by 3.3V)
# Sensitivity: 42.9 mV/A (0.0429 V/A)
# Output at 0A: 1.65V

WCS6800_SENSITIVITY = 0.0429 # V/A
WCS6800_OFFSET_VOLTAGE = 1.65 # Volts (Output at 0A current)

# --- LED Configuration ---
# UART1 RX = Red LED on GPIO 5
# UART0 TX = Blue LED on GPIO 0
# Status = Green LED on GPIO 3
red_led = machine.Pin(5, machine.Pin.OUT)
blue_led = machine.Pin(0, machine.Pin.OUT)
green_led = machine.Pin(3, machine.Pin.OUT)

# Initialize LEDs
red_led.value(0)    # Turn off initially
blue_led.value(0)   # Turn off initially
green_led.value(0)  # Turn off initially

# --- RAK3172 UART Communication Configuration ---
# RPi Pico UART0 is connected to RAK3172 UART2 (as per schematic)
# TX: GP0, RX: GP1
# Ensure these pins are not used for other purposes on the schematic

UART_ID = 0 # Use UART0
UART_TX_PIN = 0 # GP0 (connected to RAK3172 RX)
UART_RX_PIN = 1 # GP1 (connected to RAK3172 TX)
UART_BAUDRATE = 115200

uart = machine.UART(UART_ID, baudrate=UART_BAUDRATE, tx=machine.Pin(UART_TX_PIN), rx=machine.Pin(UART_RX_PIN))

# --- LoRaWAN ABP Configuration ---
# Replace with your actual ABP credentials from ChirpStack
DevAdd = "01d3257c"  # Device Address from ChirpStack
NetKey = "06ebd62a3b4e2ed8d45d38d0f515988e"  # Network Session Key from ChirpStack
SessKey = "ef54ccd9b3d974e8736c60d916ad6e96"  # Application Session Key from ChirpStack

# Setup LED and reset pin
led = machine.Pin("LED", machine.Pin.OUT) if hasattr(machine.Pin, "LED") else machine.Pin(25, machine.Pin.OUT)
rst = machine.Pin(2, machine.Pin.OUT)
rst.value(1)

def read_current():
    """Read current from WCS6800 sensor and return value in amperes"""
    # Read raw ADC value (0-65535 for 16-bit, but Pico ADC is 12-bit effective, so divide by 16)
    adc_value = adc.read_u16() / 16 # Scale to 12-bit equivalent (0-4095)

    # Convert ADC value to voltage
    voltage = (adc_value / ADC_MAX_VALUE) * ADC_REF_VOLTAGE

    # Calculate current based on WCS6800 characteristics
    # Current = (Voltage_measured - Offset_voltage) / Sensitivity
    current = (voltage - WCS6800_OFFSET_VOLTAGE) / WCS6800_SENSITIVITY

    debug_print(f"ADC raw: {adc_value:.0f}, Voltage: {voltage:.3f}V, Current: {current:.3f}A")
    return current

def clear_uart_buffer():
    """Clear any residual data in UART buffer"""
    time.sleep(0.1)
    count = 0
    while uart.any():
        data = uart.read()
        count += len(data) if data else 0
    if count > 0:
        debug_print(f"Cleared {count} bytes from UART buffer")

def send_at_command(command, expected_response="OK", timeout=5):
    """Send AT command with improved error handling and timeout"""
    clear_uart_buffer()
    
    # Turn on blue LED during transmission
    blue_led.value(1)
    
    uart.write(command + "\r\n")
    print(f"Sending command: {command}")  # Always print commands for logging
    
    start_time = time.time()
    response = ""
    
    while (time.time() - start_time) < timeout:
        if uart.any():
            data = uart.read()
            if data:
                try:
                    decoded_data = data.decode('utf-8')
                    response += decoded_data
                    debug_print(f"Response: {decoded_data.strip()}")
                    
                    # Check if we got the expected response
                    if expected_response in response:
                        blue_led.value(0)  # Turn off blue LED after successful response
                        debug_print(f"Command successful, found '{expected_response}'")
                        return True
                        
                except UnicodeDecodeError:
                    print("Error: UnicodeDecodeError in response")  # Always print errors
                    
        time.sleep(0.1)
    
    blue_led.value(0)  # Turn off blue LED after timeout
    print(f"Error: Timeout waiting for '{expected_response}' response")  # Always print errors
    return False

def wait_for_module_ready():
    """Wait for module to be ready and respond to AT commands"""
    print("Waiting for RAK3172 module to be ready...")  # Always print status
    
    # Turn on red LED to indicate waiting for module
    red_led.value(1)
    
    # Clear any boot messages
    time.sleep(3)
    clear_uart_buffer()
    
    # Try to get a response to basic AT command
    max_attempts = 5
    for attempt in range(max_attempts):
        debug_print(f"Attempt {attempt + 1} to communicate with module...")
        
        if send_at_command("AT", "OK", timeout=3):
            print("Module is ready!")  # Always print status
            red_led.value(0)  # Turn off red LED when module is ready
            return True
        
        time.sleep(1)
    
    red_led.value(0)  # Turn off red LED
    print("Error: Failed to communicate with module after multiple attempts")  # Always print errors
    return False

def initialize_lorawan():
    """Initialize the RAK module for LoRaWAN ABP mode"""
    print("Initializing LoRaWAN ABP mode...")  # Always print status
    
    # Turn on green LED during initialization
    green_led.value(1)
    
    # Wait for module to be ready
    if not wait_for_module_ready():
        green_led.value(0)  # Turn off green LED
        print("ERROR: Module not responding to AT commands")  # Always print errors
        return False
    
    # Configure LoRaWAN parameters
    commands = [
        ("AT+NJM=0", "ABP mode"),           # ABP mode
        ("AT+CLASS=C", "Class C"),          # Class C
        ("AT+BAND=3", "IN865 region"),      # IN865 region
        (f"AT+DEVADDR={DevAdd}", "Device address"),
        (f"AT+APPSKEY={SessKey}", "App session key"),
        (f"AT+NWKSKEY={NetKey}", "Network session key"),
    ]
    
    for command, description in commands:
        debug_print(f"Setting {description}...")
        if not send_at_command(command, "OK", timeout=3):
            green_led.value(0)  # Turn off green LED
            print(f"ERROR: Failed to set {description}")  # Always print errors
            return False
        time.sleep(0.5)  # Small delay between commands
    
    # Join the network
    print("Joining LoRaWAN network...")  # Always print status
    if send_at_command("AT+JOIN", "OK", timeout=10):
        print("LoRaWAN initialized and joined successfully!")  # Always print status
        # Keep green LED on to indicate successful connection
        return True
    else:
        green_led.value(0)  # Turn off green LED
        print("ERROR: Failed to join LoRaWAN network")  # Always print errors
        return False

def send_lorawan_payload(current_value):
    """Send current sensor data via LoRaWAN"""
    # Convert float current_value to a compact binary payload.
    # Use a 2-byte signed integer for current in milliAmps (mA).
    current_ma = int(current_value * 1000) # Convert Amps to milliAmps

    # Ensure value fits within int16_t range
    if current_ma > 32767: 
        debug_print(f"Current value {current_ma}mA clamped to 32767mA")
        current_ma = 32767
    if current_ma < -32768: 
        debug_print(f"Current value {current_ma}mA clamped to -32768mA")
        current_ma = -32768

    # Pack the integer into 2 bytes (big-endian)
    payload_bytes = bytearray(2)
    payload_bytes[0] = (current_ma >> 8) & 0xFF  # Most Significant Byte
    payload_bytes[1] = current_ma & 0xFF        # Least Significant Byte

    # Convert bytearray to hex string for AT command
    hex_payload = ubinascii.hexlify(payload_bytes).decode("utf-8").upper()

    # Turn on blue LED during transmission
    blue_led.value(1)
    
    # Send LoRaWAN uplink using AT+SEND command
    command = f"AT+SEND=2:{hex_payload}"
    print(f"Sending payload: {hex_payload} (Current: {current_value:.3f}A)")  # Always print status
    
    if send_at_command(command, "OK", timeout=10):
        print("Payload sent successfully")  # Always print status
        blue_led.value(0)  # Turn off blue LED after successful send
        return True
    else:
        blue_led.value(0)  # Turn off blue LED
        print("ERROR: Failed to send payload")  # Always print errors
        return False

def listen_for_downlink():
    """Listen for downlink messages and control LED"""
    print("Listening for downlink messages...")  # Always print status
    
    # Turn on red LED while listening
    red_led.value(1)
    
    timeout_count = 0
    max_timeout = 15  # Wait up to 15 seconds for downlink
    
    while timeout_count < max_timeout:
        if uart.any():
            try:
                downlink_data = uart.read().decode('utf-8').strip()
                debug_print(f"Received raw data: {downlink_data}")

                # Check for transmission done event
                if '+EVT:TX_DONE' in downlink_data:
                    debug_print("Uplink transmission confirmed")

                # Check for downlink event
                elif '+EVT:RX_C' in downlink_data or '+EVT:RX_' in downlink_data:
                    # Extract payload (last part after colons)
                    parts = downlink_data.split(':')
                    if len(parts) >= 2:
                        payload = parts[-1].strip()
                        print(f"Downlink payload received: {payload}")  # Always print status

                        # Process LED commands
                        if payload == '01':
                            print("LED ON command received")  # Always print status
                            led.value(1)
                        elif payload == '02':
                            print("LED OFF command received")  # Always print status
                            led.value(0)
                        elif payload == '04':
                            print("LED BLINK command received")  # Always print status
                            for _ in range(3):  # Blink 3 times
                                led.value(1)
                                time.sleep(0.3)
                                led.value(0)
                                time.sleep(0.3)
                        else:
                            print(f"Unknown command: {payload}")  # Always print status
                    
                    red_led.value(0)  # Turn off red LED after processing downlink
                    return True
                    
            except UnicodeDecodeError:
                print("Error: UnicodeDecodeError in downlink data")  # Always print errors
        
        time.sleep(1)
        timeout_count += 1
        if timeout_count % 5 == 0:  # Log every 5 seconds
            debug_print(f"Waiting for downlink... {timeout_count}/{max_timeout}s")
    
    red_led.value(0)  # Turn off red LED after timeout
    debug_print("No downlink received within timeout period")
    return False

def main():
    """Main function"""
    print("=" * 60)
    print("WCS6800 Current Sensor LoRaWAN Monitor")
    print("=" * 60)
    print(f"Debug mode: {'Enabled' if DEBUG else 'Disabled'}")
    print(f"UART: TX=GP{UART_TX_PIN}, RX=GP{UART_RX_PIN} @ {UART_BAUDRATE} baud")
    print(f"ADC: WCS6800 on GP{ADC_PIN}")
    print(f"Device Address: {DevAdd}")
    print(f"LED Indicators: Red=GP5, Blue=GP0, Green=GP3")
    print("=" * 60)
    
    # Initial LED test - blink all LEDs once
    debug_print("Running LED test sequence")
    for led_pin in [red_led, green_led, blue_led, led]:
        led_pin.value(1)
        time.sleep(0.2)
        led_pin.value(0)
        time.sleep(0.2)
    
    # Initialize LoRaWAN
    if not initialize_lorawan():
        # Blink red LED to indicate initialization failure
        for _ in range(5):
            red_led.value(1)
            time.sleep(0.2)
            red_led.value(0)
            time.sleep(0.2)
        print("FATAL ERROR: LoRaWAN initialization failed")  # Always print errors
        return
    
    print("Starting main monitoring loop...")  # Always print status
    
    # Main monitoring loop
    cycle_count = 0
    while True:
        try:
            cycle_count += 1
            debug_print(f"\n--- Measurement cycle #{cycle_count} ---")
            
            # Read current from sensor
            current_val = read_current()
            print(f"\nCurrent Reading: {current_val:.3f} A")  # Always print measurements
            
            # Send data via LoRaWAN
            if send_lorawan_payload(current_val):
                # Listen for downlink commands
                listen_for_downlink()
            else:
                debug_print("Skipping downlink listen due to send failure")
            
        except Exception as e:
            print(f"ERROR in main loop: {e}")  # Always print errors
            
            # Indicate error with rapid red LED blinks
            for _ in range(3):
                red_led.value(1)
                time.sleep(0.1)
                red_led.value(0)
                time.sleep(0.1)
        
        # Indicate we're in wait period with pulsing green LED
        print(f"Waiting 60 seconds before next transmission...")  # Always print status
        for i in range(6):  # Pulse for 10 seconds, 6 times
            debug_print(f"Wait period {(i+1)*10}/60 seconds")
            green_led.value(1)
            time.sleep(1)
            green_led.value(0)
            time.sleep(9)

# Run the main function
if __name__ == '__main__':
    main()