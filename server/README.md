# OTA Server Components

This directory contains the server-side components for the OTA update system.

## Files

### `ota_backend.php`
PHP backend script that serves firmware files to devices. Features:
- Auto-detects correct server paths
- Logs all download requests
- Supports version management
- CORS headers for IoT devices

### `simple_ota_upload.py`
Python script to upload firmware to your server. Features:
- FTP upload support
- Manual upload instructions
- File verification
- Progress tracking

## Setup

1. Upload `ota_backend.php` to your server as `index.php` in the `/ota/` directory
2. Use `simple_ota_upload.py` to upload firmware files
3. Test the OTA URL: `http://iotlogic.in/ota/firmware.bin`

## Usage

```bash
# Upload firmware to server
python simple_ota_upload.py

# Test OTA URL
curl -I http://iotlogic.in/ota/firmware.bin
```
