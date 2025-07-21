#include "ota_manager.h"
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <LittleFS.h>

// Global variables
uint8_t otaStatus = OTA_STATUS_IDLE;
uint8_t otaProgress = 0;
unsigned long otaStartTime = 0;
bool otaInProgress = false;

// Setup OTA functionality
bool setupOTA() {
    if (!OTA_ENABLED) {
        log_info("OTA updates disabled in configuration");
        return false;
    }
    
    log_info("OTA update system initialized");
    otaStatus = OTA_STATUS_IDLE;
    otaInProgress = false;
    
    return true;
}

// Trigger OTA update with specified method
bool triggerOTAUpdate(uint8_t method) {
    if (!OTA_ENABLED) {
        log_error("OTA updates are disabled");
        return false;
    }
    
    if (otaInProgress) {
        log_error("OTA update already in progress");
        return false;
    }
    
    otaInProgress = true;
    otaStartTime = millis();
    
    switch (method) {
        case OTA_METHOD_WIFI:
            log_info("Triggering WiFi OTA update");
            otaStatus = OTA_STATUS_WIFI;
            // WiFi OTA will be handled in loop via handleOTAEvents
            return true;
            
        case OTA_METHOD_BLE:
            log_info("BLE OTA update not yet implemented");
            otaStatus = OTA_STATUS_FAILED;
            otaInProgress = false;
            return false;
            
        default:
            log_format(LOG_ERROR, "Unknown OTA method: %d", method);
            otaStatus = OTA_STATUS_FAILED;
            otaInProgress = false;
            return false;
    }
}

// Download and apply OTA update via WiFi
bool downloadAndApplyOTA() {
    log_info("Starting WiFi OTA update process");
    otaStatus = OTA_STATUS_DOWNLOAD;
    
    if (WiFi.status() != WL_CONNECTED) {
        log_error("WiFi not connected, OTA update failed");
        otaStatus = OTA_STATUS_FAILED;
        otaInProgress = false;
        return false;
    }
    
    log_format(LOG_INFO, "Downloading firmware from: %s", OTA_SERVER_URL);
    
    HTTPClient http;
    http.begin(OTA_SERVER_URL);
    
    // Use HTTP authentication if credentials are provided
    if (strlen(OTA_HTTP_USERNAME) > 0 && strlen(OTA_HTTP_PASSWORD) > 0) {
        http.setAuthorization(OTA_HTTP_USERNAME, OTA_HTTP_PASSWORD);
    }
    
    // Get file size
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        log_format(LOG_ERROR, "HTTP GET failed, error: %d", httpCode);
        http.end();
        otaStatus = OTA_STATUS_FAILED;
        otaInProgress = false;
        return false;
    }
    
    int contentLength = http.getSize();
    if (contentLength <= 0) {
        log_error("Invalid content length");
        http.end();
        otaStatus = OTA_STATUS_FAILED;
        otaInProgress = false;
        return false;
    }
    
    log_format(LOG_INFO, "Firmware size: %d bytes", contentLength);
    
    // For Pico W, we'll download firmware to LittleFS and then apply it
    log_info("Starting firmware download for Pico W...");

    // Initialize LittleFS if not already done
    if (!LittleFS.begin()) {
        log_error("Failed to initialize LittleFS");
        http.end();
        otaStatus = OTA_STATUS_FAILED;
        otaInProgress = false;
        return false;
    }

    // Create firmware file on LittleFS
    File firmwareFile = LittleFS.open("/firmware.bin", "w");
    if (!firmwareFile) {
        log_error("Failed to create firmware file");
        http.end();
        otaStatus = OTA_STATUS_FAILED;
        otaInProgress = false;
        return false;
    }

    // Get WiFi stream for download
    WiFiClient* stream = http.getStreamPtr();
    if (!stream) {
        log_error("Failed to get HTTP stream");
        firmwareFile.close();
        http.end();
        otaStatus = OTA_STATUS_FAILED;
        otaInProgress = false;
        return false;
    }

    // Download firmware in chunks
    const size_t bufferSize = 1024;
    uint8_t buffer[bufferSize];
    size_t totalDownloaded = 0;

    log_info("Downloading firmware to flash storage...");

    while (totalDownloaded < contentLength) {
        size_t availableBytes = stream->available();
        if (availableBytes > 0) {
            size_t readSize = min(availableBytes, bufferSize);
            size_t bytesRead = stream->readBytes(buffer, readSize);

            if (bytesRead > 0) {
                size_t written = firmwareFile.write(buffer, bytesRead);
                if (written != bytesRead) {
                    log_format(LOG_ERROR, "File write failed: expected %d, wrote %d", bytesRead, written);
                    break;
                }

                totalDownloaded += bytesRead;

                // Log progress every 10KB
                if (totalDownloaded % 10240 == 0 || totalDownloaded == contentLength) {
                    int progress = (totalDownloaded * 100) / contentLength;
                    log_format(LOG_INFO, "Download progress: %d%% (%d/%d bytes)",
                              progress, totalDownloaded, contentLength);
                }
            }
        } else {
            delay(10); // Small delay if no data available
        }

        // Timeout check
        if (millis() - otaStartTime > 300000) { // 5 minute timeout
            log_error("OTA download timeout");
            break;
        }
    }

    firmwareFile.close();
    http.end();

    if (totalDownloaded != contentLength) {
        log_format(LOG_ERROR, "Download incomplete: %d/%d bytes", totalDownloaded, contentLength);
        LittleFS.remove("/firmware.bin");
        otaStatus = OTA_STATUS_FAILED;
        otaInProgress = false;
        return false;
    }

    log_info("Firmware download completed successfully!");
    log_format(LOG_INFO, "Downloaded %d bytes to /firmware.bin", totalDownloaded);

    // For Pico W, we need to implement the actual flash update
    // This is a simplified version - in production you'd want proper verification
    otaStatus = OTA_STATUS_VERIFY;
    log_info("Firmware verification (basic file check)");

    // Check if file exists and has correct size
    File verifyFile = LittleFS.open("/firmware.bin", "r");
    if (verifyFile && verifyFile.size() == contentLength) {
        log_info("Firmware file verification passed");
        verifyFile.close();

        // Mark for update on next boot (this is platform-specific)
        log_info("OTA update prepared successfully!");
        log_warning("IMPORTANT: Pico W OTA requires manual reboot or bootloader integration");
        log_info("Firmware is ready at /firmware.bin");
        log_info("For complete OTA, implement bootloader integration or use picotool");

        otaStatus = OTA_STATUS_SUCCESS;

        // Optional: Reboot to apply update (if bootloader supports it)
        log_info("Rebooting in 5 seconds...");
        delay(5000);
        rp2040.reboot();

        return true;
    } else {
        log_error("Firmware verification failed");
        if (verifyFile) verifyFile.close();
        LittleFS.remove("/firmware.bin");
        otaStatus = OTA_STATUS_FAILED;
        otaInProgress = false;
        return false;
    }
    otaStatus = OTA_STATUS_SUCCESS;
    otaInProgress = false;
    
    // Don't actually reboot in this simulated version
    return true;
}

// Handle OTA update events
void handleOTAEvents() {
    if (!OTA_ENABLED || !otaInProgress) {
        return;
    }
    
    // Handle OTA states
    switch (otaStatus) {
        case OTA_STATUS_WIFI:
            // Ensure WiFi is connected before proceeding
            if (WiFi.status() == WL_CONNECTED) {
                // Start the download in the next iteration
                otaStatus = OTA_STATUS_DOWNLOAD;
            } else {
                log_error("WiFi connection failed during OTA");
                otaStatus = OTA_STATUS_FAILED;
                otaInProgress = false;
            }
            break;
            
        case OTA_STATUS_DOWNLOAD:
            // Start download and apply process
            downloadAndApplyOTA();
            break;
            
        case OTA_STATUS_FAILED:
            // Reset OTA state after timeout
            if (millis() - otaStartTime > 30000) { // 30 seconds timeout
                log_info("Resetting OTA status after failure");
                otaStatus = OTA_STATUS_IDLE;
                otaInProgress = false;
            }
            break;
            
        default:
            // Other states are handled in downloadAndApplyOTA
            break;
    }
}

// Get current OTA status
uint8_t getOTAStatus() {
    return otaStatus;
}

// Verify firmware integrity (placeholder for actual verification)
bool verifyFirmware(uint8_t* buffer, size_t size) {
    // This is a placeholder for custom verification if needed
    log_info("Firmware verification placeholder");
    return true;
} 