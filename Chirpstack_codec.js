// WCS6800 Current Sensor Codec for ChirpStack
// This codec handles decoding uplink current sensor data and encoding downlink LED commands

// Decode uplink function
function decodeUplink(input) {
    var data = {};
    var warnings = [];
    var errors = [];

    try {
        // Check if we have at least 2 bytes for current data
        if (input.bytes.length < 2) {
            errors.push("Payload too short, expected at least 2 bytes");
            return {
                data: data,
                warnings: warnings,
                errors: errors
            };
        }

        // Decode current data (2 bytes, big-endian, signed integer in milliAmps)
        var currentRaw = (input.bytes[0] << 8) | input.bytes[1];
        
        // Convert from unsigned to signed 16-bit integer
        if (currentRaw > 32767) {
            currentRaw = currentRaw - 65536;
        }
        
        // Convert from milliAmps to Amps
        var currentAmps = currentRaw / 1000.0;
        
        // Add to data object
        data.current_ma = currentRaw;        // Current in milliAmps
        data.current_a = currentAmps;        // Current in Amps
        data.current_formatted = currentAmps.toFixed(3) + " A";  // Formatted string
        
        // Add sensor information
        data.sensor_type = "WCS6800";
        data.measurement_type = "AC Current";
        
        // Add warnings for extreme values
        if (Math.abs(currentAmps) > 30) {
            warnings.push("Current value exceeds typical WCS6800 range (±30A)");
        }
        
        if (Math.abs(currentAmps) < 0.1) {
            data.status = "Low current or no load detected";
        } else if (currentAmps > 20) {
            data.status = "High current detected";
            warnings.push("High current detected - check load");
        } else {
            data.status = "Normal operation";
        }
        
        // Add timestamp
        data.timestamp = new Date().toISOString();
        
    } catch (e) {
        errors.push("Error decoding payload: " + e.message);
    }

    return {
        data: data,
        warnings: warnings,
        errors: errors
    };
}

// Encode downlink function
function encodeDownlink(input) {
    var warnings = [];
    var errors = [];
    var bytes = [];

    try {
        // Check if LED command is provided
        if (input.data.led_command !== undefined) {
            var command = input.data.led_command.toLowerCase();
            
            switch (command) {
                case "on":
                case "turn_on":
                case "1":
                    bytes = [0x01];
                    break;
                    
                case "off":
                case "turn_off":
                case "0":
                    bytes = [0x02];
                    break;
                    
                case "blink":
                case "flash":
                case "toggle":
                    bytes = [0x04];
                    break;
                    
                default:
                    errors.push("Invalid LED command. Use 'on', 'off', or 'blink'");
                    return {
                        bytes: bytes,
                        warnings: warnings,
                        errors: errors
                    };
            }
        }
        
        // Check if raw command is provided (for advanced users)
        else if (input.data.raw_command !== undefined) {
            if (typeof input.data.raw_command === 'number') {
                if (input.data.raw_command >= 0 && input.data.raw_command <= 255) {
                    bytes = [input.data.raw_command];
                } else {
                    errors.push("Raw command must be between 0 and 255");
                }
            } else {
                errors.push("Raw command must be a number");
            }
        }
        
        // No valid command provided
        else {
            errors.push("No valid command provided. Use 'led_command' or 'raw_command'");
        }
        
    } catch (e) {
        errors.push("Error encoding downlink: " + e.message);
    }

    return {
        bytes: bytes,
        warnings: warnings,
        errors: errors
    };
}

// Test functions (for development and debugging)
function testDecoder() {
    // Test case 1: 5.5A current (5500 mA)
    var test1 = decodeUplink({
        bytes: [0x15, 0x7C],  // 5500 in hex
        fPort: 2
    });
    console.log("Test 1 (5.5A):", JSON.stringify(test1, null, 2));
    
    // Test case 2: -2.5A current (-2500 mA)
    var test2 = decodeUplink({
        bytes: [0xF6, 0x3C],  // -2500 in hex (two's complement)
        fPort: 2
    });
    console.log("Test 2 (-2.5A):", JSON.stringify(test2, null, 2));
    
    // Test case 3: 0.1A current (100 mA)
    var test3 = decodeUplink({
        bytes: [0x00, 0x64],  // 100 in hex
        fPort: 2
    });
    console.log("Test 3 (0.1A):", JSON.stringify(test3, null, 2));
}

function testEncoder() {
    // Test LED ON command
    var test1 = encodeDownlink({
        data: { led_command: "on" }
    });
    console.log("Test LED ON:", JSON.stringify(test1, null, 2));
    
    // Test LED OFF command
    var test2 = encodeDownlink({
        data: { led_command: "off" }
    });
    console.log("Test LED OFF:", JSON.stringify(test2, null, 2));
    
    // Test LED BLINK command
    var test3 = encodeDownlink({
        data: { led_command: "blink" }
    });
    console.log("Test LED BLINK:", JSON.stringify(test3, null, 2));
    
    // Test raw command
    var test4 = encodeDownlink({
        data: { raw_command: 0x04 }
    });
    console.log("Test RAW command:", JSON.stringify(test4, null, 2));
}

// Uncomment the following lines to run tests
// testDecoder();
// testEncoder();

// Example usage in ChirpStack:
/*
Uplink example:
- Payload: 157C (hex) = 5500 decimal = 5.5A current
- Decoded data will show: current_a: 5.5, current_ma: 5500, status: "Normal operation"

Downlink examples:
- To turn LED ON: {"led_command": "on"}
- To turn LED OFF: {"led_command": "off"}  
- To blink LED: {"led_command": "blink"}
- Raw command: {"raw_command": 4}
*/