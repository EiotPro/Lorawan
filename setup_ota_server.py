#!/usr/bin/env python3
"""
Simple OTA Server Setup Script for LoRaWAN Device
This script helps set up a simple HTTP server for OTA firmware updates.
"""

import os
import sys
import shutil
import socket
import http.server
import socketserver
from pathlib import Path

def get_local_ip():
    """Get the local IP address of this machine."""
    try:
        # Connect to a remote address to determine local IP
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except Exception:
        return "127.0.0.1"

def find_firmware_file():
    """Find the compiled firmware file."""
    possible_paths = [
        ".pio/build/rpipicow/firmware.bin",
        ".pio\\build\\rpipicow\\firmware.bin",
        "firmware.bin"
    ]
    
    for path in possible_paths:
        if os.path.exists(path):
            return path
    return None

def setup_server_directory():
    """Set up the server directory with firmware file."""
    print("Setting up OTA server directory...")
    
    # Create server directory
    server_dir = Path("ota_server")
    server_dir.mkdir(exist_ok=True)
    
    # Find firmware file
    firmware_path = find_firmware_file()
    if not firmware_path:
        print("❌ ERROR: Could not find firmware.bin file!")
        print("Please compile your project first with: pio run -e rpipicow")
        return None
    
    # Copy firmware to server directory
    dest_path = server_dir / "firmware.bin"
    shutil.copy2(firmware_path, dest_path)
    
    print(f"✅ Copied firmware from: {firmware_path}")
    print(f"✅ Server directory created: {server_dir}")
    print(f"✅ Firmware available at: {dest_path}")
    
    return server_dir

def update_config_file(server_ip, port=8082):
    """Update the config.h file with the correct OTA server URL."""
    config_path = Path("include/config.h")
    
    if not config_path.exists():
        print(f"❌ WARNING: Could not find {config_path}")
        return False
    
    # Read the config file
    with open(config_path, 'r') as f:
        content = f.read()
    
    # Update the OTA server URL
    new_url = f"http://{server_ip}:{port}/firmware.bin"
    
    # Replace the OTA_SERVER_URL line
    lines = content.split('\n')
    updated = False
    for i, line in enumerate(lines):
        if 'OTA_SERVER_URL' in line and '=' in line:
            lines[i] = f'const char* const OTA_SERVER_URL = "{new_url}";'
            updated = True
            break
    
    if not updated:
        print(f"❌ WARNING: Could not find OTA_SERVER_URL in config.h")
        print(f"Please manually add: const char* const OTA_SERVER_URL = \"{new_url}\";")
        return False
    
    # Write back to file
    with open(config_path, 'w') as f:
        f.write('\n'.join(lines))
    
    print(f"✅ Updated config.h with URL: {new_url}")
    return True

def start_server(server_dir, port=8082):
    """Start the HTTP server."""
    os.chdir(server_dir)
    
    class CustomHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
        def log_message(self, format, *args):
            print(f"[{self.address_string()}] {format % args}")
        
        def end_headers(self):
            # Add CORS headers
            self.send_header('Access-Control-Allow-Origin', '*')
            self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
            self.send_header('Access-Control-Allow-Headers', 'Content-Type')
            super().end_headers()
    
    try:
        with socketserver.TCPServer(("", port), CustomHTTPRequestHandler) as httpd:
            print(f"\n🚀 OTA Server started successfully!")
            print(f"📁 Serving files from: {server_dir.absolute()}")
            print(f"🌐 Server URL: http://{get_local_ip()}:{port}/")
            print(f"📦 Firmware URL: http://{get_local_ip()}:{port}/firmware.bin")
            print(f"\n📋 Next steps:")
            print(f"1. Compile and upload your updated firmware: pio run -t upload -e rpipicow")
            print(f"2. Send downlink command '03' from ChirpStack to trigger OTA")
            print(f"3. Monitor device serial output for OTA progress")
            print(f"\n⏹️  Press Ctrl+C to stop the server")
            
            httpd.serve_forever()
            
    except KeyboardInterrupt:
        print(f"\n🛑 Server stopped by user")
    except OSError as e:
        if e.errno == 98 or "already in use" in str(e):
            print(f"❌ ERROR: Port {port} is already in use!")
            print(f"Try using a different port or stop the existing server.")
        else:
            print(f"❌ ERROR: Could not start server: {e}")

def main():
    print("🔧 LoRaWAN OTA Server Setup")
    print("=" * 40)
    
    # Get local IP
    local_ip = get_local_ip()
    port = 8082
    
    print(f"🖥️  Local IP: {local_ip}")
    print(f"🔌 Port: {port}")
    
    # Set up server directory
    server_dir = setup_server_directory()
    if not server_dir:
        sys.exit(1)
    
    # Update config file
    update_config_file(local_ip, port)
    
    print(f"\n⚠️  IMPORTANT: You need to recompile and upload your firmware")
    print(f"   to use the new OTA server URL!")
    print(f"   Run: pio run -t upload -e rpipicow")
    
    # Ask user if they want to start the server
    response = input(f"\n❓ Start OTA server now? (y/n): ").lower().strip()
    if response in ['y', 'yes']:
        start_server(server_dir, port)
    else:
        print(f"\n📝 To start the server later, run:")
        print(f"   cd {server_dir}")
        print(f"   python -m http.server {port}")

if __name__ == "__main__":
    main()
