# LoRaWAN Downlink Testing Guide - ENHANCED VERSION

## Compilation Status ✅
The code has been successfully compiled with major downlink improvements:
- ✅ PIN redefinition warnings fixed
- ✅ OTA function naming conflict resolved
- ✅ All dependencies properly linked
- ✅ **NEW**: Completely rewritten downlink handling system
- ✅ **NEW**: Character-by-character UART reading for better reliability
- ✅ **NEW**: Improved AT+RECV command usage
- ✅ **NEW**: Built-in downlink system test on startup

## Upload and Test Instructions

### 1. Upload the Firmware
```bash
pio run -t upload -e rpipicow
```

### 2. Monitor Serial Output
```bash
pio device monitor -b 115200
```

### 3. Expected Startup Sequence
Look for these key messages in the serial output:
```
[INFO] WCS6800 Current Sensor LoRaWAN Monitor
[INFO] Setting ABP mode...
[INFO] Setting Class C...
[INFO] Setting IN865 region...
[INFO] Setting device address...
[INFO] Setting app session key...
[INFO] Setting network session key...
[INFO] Joining LoRaWAN network...
[INFO] Successfully joined LoRaWAN network!
[INFO] Class C confirmed active
[INFO] LoRaWAN initialized and joined successfully!
[INFO] Starting main monitoring loop...
[INFO] === Testing Downlink System ===
[INFO] Test 1: Checking AT+RECV command...
[INFO] ✓ No pending downlink data (normal)
[INFO] Test 2: Verifying Class C mode...
[INFO] ✓ Class C mode confirmed
[INFO] Test 3: Checking join status...
[INFO] ✓ Device is joined to network
[INFO] Test 4: Testing LED functionality...
[INFO] ✓ LED test completed
[INFO] === Downlink System Test Complete ===
```

### 4. Test Downlink Commands

From your ChirpStack console, send these test downlinks on **port 2**:

#### Test 1: LED ON
- **Payload (Hex)**: `01`
- **Expected Response**: 
  ```
  [INFO] Downlink event detected!
  [INFO] Received downlink on port 2: 01
  [INFO] Processing downlink payload: 01
  [INFO] LED ON command received
  ```
- **Physical Result**: LED on GPIO 25 should turn ON

#### Test 2: LED OFF  
- **Payload (Hex)**: `02`
- **Expected Response**:
  ```
  [INFO] Downlink event detected!
  [INFO] Received downlink on port 2: 02
  [INFO] Processing downlink payload: 02
  [INFO] LED OFF command received
  ```
- **Physical Result**: LED on GPIO 25 should turn OFF

#### Test 3: LED Blink Test
- **Payload (Hex)**: `04`
- **Expected Response**:
  ```
  [INFO] Downlink event detected!
  [INFO] Received downlink on port 2: 04
  [INFO] Processing downlink payload: 04
  [INFO] LED BLINK command received
  ```
- **Physical Result**: LED should blink 5 times (200ms on/off)

#### Test 4: OTA Update (if enabled)
- **Payload (Hex)**: `03`
- **Expected Response**:
  ```
  [INFO] Downlink event detected!
  [INFO] Received downlink on port 2: 03
  [INFO] Processing downlink payload: 03
  [INFO] OTA UPDATE command received
  [INFO] Handling OTA update command from downlink
  ```

### 5. Continuous Monitoring

The device now continuously monitors for downlinks:
- **Immediate check**: After each uplink transmission
- **Periodic check**: Every 5 seconds using `AT+RECV=?`
- **Class C listening**: Continuous reception window

Look for these periodic messages:
```
[DEBUG] Pending downlink check response: 0:
```
This indicates the system is actively checking for downlinks.

### 6. Troubleshooting

#### No Downlinks Received
1. **Check Network Server**:
   - Verify device is active in ChirpStack
   - Ensure Class C is enabled for the device
   - Check downlink queue status

2. **Verify Device Configuration**:
   - Device Address: `01d3257c`
   - Network Session Key: `06ebd62a3b4e2ed8d45d38d0f515988e`
   - App Session Key: `ef54ccd9b3d974e8736c60d916ad6e96`

3. **Check Serial Logs**:
   ```
   [DEBUG] RAW received: '+EVT:RX_1, RSSI -52, SNR 9'
   [DEBUG] AT+RECV response: 2:01
   ```

4. **Gateway Issues**:
   - Ensure gateway is online and connected
   - Check gateway logs for downlink transmission
   - Verify device is within range

#### Partial Downlink Reception
If you see downlink events but no payload:
```
[INFO] Downlink event detected!
[INFO] Downlink RSSI: -52 dBm
[INFO] Downlink SNR: 9
[WARNING] Empty payload received
```

This indicates the downlink frame was received but contained no data.

### 7. Advanced Testing

#### Custom Commands
Add your own commands by modifying `processDownlinkPayload()`:
```cpp
else if (payload == "05") {
  log_info("CUSTOM COMMAND received");
  // Your custom logic here
}
```

#### Multi-byte Payloads
Test with longer payloads:
- **Payload (Hex)**: `AABBCCDD`
- **Expected**: Multi-byte payload processing

### 8. Performance Monitoring

Monitor these metrics:
- **Uplink Success Rate**: Should remain high
- **Downlink Reception**: Check RSSI/SNR values
- **Memory Usage**: Watch for any memory leaks
- **Timing**: Ensure uplink intervals are maintained

### 9. Next Steps

Once basic downlink reception is confirmed:
1. **Implement additional commands** as needed
2. **Optimize polling intervals** based on your requirements
3. **Add downlink acknowledgments** if required
4. **Test with production network server**

## Success Criteria

✅ Device joins LoRaWAN network successfully  
✅ Class C mode is confirmed active  
✅ Uplinks are transmitted regularly  
✅ Downlink events are detected and logged  
✅ LED commands work correctly  
✅ Continuous monitoring is active  

If all criteria are met, your downlink reception is working correctly!
