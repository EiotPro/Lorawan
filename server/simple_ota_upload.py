#!/usr/bin/env python3
"""
Simple OTA Upload Script for iotlogic.in
Works with cPanel hosting and FTP
"""

import os
import sys
from pathlib import Path
import hashlib
import getpass

# Configuration for your hosting
SERVER_DOMAIN = "iotlogic.in"
FTP_PORT = 21
REMOTE_DIR = "public_html/ota"
OTA_URL = f"http://{SERVER_DOMAIN}/ota/firmware.bin"

def calculate_file_hash(file_path):
    """Calculate MD5 hash of firmware file."""
    hash_md5 = hashlib.md5()
    with open(file_path, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest()

def upload_via_ftp(username, password):
    """Upload firmware using FTP."""
    firmware_path = Path("../.pio/build/rpipicow/firmware.bin")
    
    if not firmware_path.exists():
        print("❌ Firmware file not found! Please compile first:")
        print("   pio run -e rpipicow")
        return False
    
    # Get file info
    file_size = firmware_path.stat().st_size
    file_hash = calculate_file_hash(firmware_path)
    
    print(f"📤 Uploading firmware via FTP...")
    print(f"   File: {firmware_path}")
    print(f"   Size: {file_size:,} bytes ({file_size/1024:.1f} KB)")
    print(f"   Hash: {file_hash}")
    print(f"   Server: {SERVER_DOMAIN}:{FTP_PORT}")
    
    try:
        from ftplib import FTP
        
        # Connect to FTP
        ftp = FTP()
        ftp.connect(SERVER_DOMAIN, FTP_PORT)
        ftp.login(username, password)
        
        print("✅ FTP connection successful!")
        
        # Navigate to public_html
        try:
            ftp.cwd('public_html')
        except:
            print("❌ Cannot access public_html directory")
            ftp.quit()
            return False
        
        # Create ota directory if it doesn't exist
        try:
            ftp.mkd('ota')
            print("📁 Created ota directory")
        except:
            print("📁 ota directory already exists")
        
        # Change to ota directory
        ftp.cwd('ota')
        
        # Upload firmware file
        print("📤 Uploading firmware.bin...")
        with open(firmware_path, 'rb') as f:
            ftp.storbinary('STOR firmware.bin', f)
        
        # Upload PHP backend if it exists
        backend_path = Path("ota_backend.php")
        if backend_path.exists():
            print("📤 Uploading PHP backend...")
            with open(backend_path, 'rb') as f:
                ftp.storbinary('STOR index.php', f)
        
        ftp.quit()
        print("✅ Upload completed successfully!")
        print(f"🌐 OTA URL: {OTA_URL}")
        return True
        
    except ImportError:
        print("❌ ftplib not available")
        return False
    except Exception as e:
        print(f"❌ FTP upload failed: {e}")
        return False

def test_ota_url():
    """Test if the OTA URL is accessible."""
    print(f"🔍 Testing OTA URL: {OTA_URL}")
    try:
        import requests
        response = requests.head(OTA_URL, timeout=10)
        if response.status_code == 200:
            print("✅ OTA URL is accessible!")
            content_length = response.headers.get('content-length')
            if content_length:
                print(f"   File size: {int(content_length):,} bytes")
            return True
        else:
            print(f"❌ OTA URL returned status: {response.status_code}")
            return False
    except ImportError:
        print("💡 Install requests library for URL testing: pip install requests")
        return True
    except Exception as e:
        print(f"❌ Failed to access OTA URL: {e}")
        return False

def show_manual_instructions():
    """Show manual upload instructions."""
    firmware_path = Path("../.pio/build/rpipicow/firmware.bin")
    print("\n💡 Manual Upload Instructions:")
    print("="*50)
    print("📁 Files to upload:")
    print(f"   Local: {firmware_path}")
    print(f"   Remote: public_html/ota/firmware.bin")
    print()
    print("🔧 Using FileZilla/FTP Client:")
    print(f"   Host: {SERVER_DOMAIN}")
    print(f"   Port: {FTP_PORT}")
    print(f"   Username: [your FTP username]")
    print(f"   Password: [your FTP password]")
    print(f"   Directory: public_html/ota/")
    print()
    print("🌐 Using cPanel File Manager:")
    print(f"   1. Login to cPanel at {SERVER_DOMAIN}/cpanel")
    print("   2. Open File Manager")
    print("   3. Navigate to public_html")
    print("   4. Create folder 'ota' if it doesn't exist")
    print("   5. Upload firmware.bin to the ota folder")
    print()
    print("📋 PHP Backend:")
    print("   Also upload ota_backend.php as index.php in the ota folder")

def main():
    print("🚀 Simple OTA Upload for iotlogic.in")
    print("=" * 40)
    
    # Check if this is a PlatformIO project
    if not Path("../platformio.ini").exists():
        print("❌ This doesn't appear to be a PlatformIO project!")
        print("💡 Run this script from the server/ directory")
        return
    
    # Check if firmware exists
    firmware_path = Path("../.pio/build/rpipicow/firmware.bin")
    if not firmware_path.exists():
        print("❌ Firmware not found! Compile first:")
        print("   pio run -e rpipicow")
        return
    
    print("📋 Upload Options:")
    print("1. FTP Upload (automatic)")
    print("2. Manual upload instructions")
    print("3. Test OTA URL only")
    
    choice = input("\nChoose option (1-3): ").strip()
    
    if choice == "1":
        print(f"\n📤 FTP Upload to {SERVER_DOMAIN}")
        username = input("Enter FTP username: ").strip()
        if not username:
            print("❌ Username required")
            return
        
        password = getpass.getpass("Enter FTP password: ")
        if not password:
            print("❌ Password required")
            return
        
        if upload_via_ftp(username, password):
            # Test the URL after upload
            test_ota_url()
            print("\n🎯 Next Steps:")
            print("1. Send OTA command from ChirpStack: FPort=2, Payload=03")
            print("2. Monitor device serial output")
        else:
            show_manual_instructions()
    
    elif choice == "2":
        show_manual_instructions()
    
    elif choice == "3":
        test_ota_url()
    
    else:
        print("❌ Invalid choice")

if __name__ == "__main__":
    main()
