<?php
/**
 * LoRaWAN OTA Firmware Update Backend
 * PHP script to handle firmware downloads for IoT devices
 */

// Configuration - Auto-detect the correct paths
$DOCUMENT_ROOT = $_SERVER['DOCUMENT_ROOT'];
$FIRMWARE_DIR = $DOCUMENT_ROOT . '/ota/';
$LOG_FILE = $DOCUMENT_ROOT . '/ota/ota_log.txt';
$ALLOWED_DEVICES = []; // Empty = allow all devices

// Create directories if they don't exist
if (!is_dir($FIRMWARE_DIR)) {
    mkdir($FIRMWARE_DIR, 0755, true);
}

// Enable CORS for IoT devices
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

// Handle preflight requests
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit();
}

/**
 * Log OTA requests for monitoring
 */
function logOTARequest($device_id, $action, $status, $details = '') {
    global $LOG_FILE;
    
    $timestamp = date('Y-m-d H:i:s');
    $ip = $_SERVER['REMOTE_ADDR'] ?? 'unknown';
    $user_agent = $_SERVER['HTTP_USER_AGENT'] ?? 'unknown';
    
    $log_entry = sprintf(
        "[%s] Device: %s | Action: %s | Status: %s | IP: %s | UA: %s | Details: %s\n",
        $timestamp, $device_id, $action, $status, $ip, $user_agent, $details
    );
    
    // Create log directory if it doesn't exist
    $log_dir = dirname($LOG_FILE);
    if (!is_dir($log_dir)) {
        mkdir($log_dir, 0755, true);
    }
    
    // Try to write log, but don't fail if it can't
    if (is_writable($log_dir) || is_writable($LOG_FILE)) {
        file_put_contents($LOG_FILE, $log_entry, FILE_APPEND | LOCK_EX);
    }
}

/**
 * Get device ID from request headers or parameters
 */
function getDeviceId() {
    // Try to get device ID from various sources
    $device_id = $_GET['device_id'] ?? 
                 $_SERVER['HTTP_X_DEVICE_ID'] ?? 
                 $_SERVER['HTTP_USER_AGENT'] ?? 
                 'unknown';
    
    return preg_replace('/[^a-zA-Z0-9_-]/', '', $device_id);
}

/**
 * Check if device is authorized for OTA updates
 */
function isDeviceAuthorized($device_id) {
    global $ALLOWED_DEVICES;
    
    // If no restrictions, allow all devices
    if (empty($ALLOWED_DEVICES)) {
        return true;
    }
    
    return in_array($device_id, $ALLOWED_DEVICES);
}

/**
 * Get firmware file path
 */
function getFirmwarePath($version = 'latest') {
    global $FIRMWARE_DIR;
    
    // Default firmware file
    $firmware_file = $FIRMWARE_DIR . 'firmware.bin';
    
    // Version-specific firmware (optional)
    if ($version !== 'latest') {
        $version_file = $FIRMWARE_DIR . "firmware_v{$version}.bin";
        if (file_exists($version_file)) {
            $firmware_file = $version_file;
        }
    }
    
    return $firmware_file;
}

/**
 * Serve firmware file with proper headers
 */
function serveFirmware($firmware_path, $device_id) {
    if (!file_exists($firmware_path)) {
        logOTARequest($device_id, 'download', 'error', 'Firmware file not found');
        http_response_code(404);
        echo json_encode(['error' => 'Firmware not found']);
        return;
    }
    
    $file_size = filesize($firmware_path);
    $file_hash = md5_file($firmware_path);
    
    // Set headers for firmware download
    header('Content-Type: application/octet-stream');
    header('Content-Disposition: attachment; filename="firmware.bin"');
    header('Content-Length: ' . $file_size);
    header('Cache-Control: no-cache, no-store, must-revalidate');
    header('Pragma: no-cache');
    header('Expires: 0');
    header('X-Firmware-Size: ' . $file_size);
    header('X-Firmware-Hash: ' . $file_hash);
    
    logOTARequest($device_id, 'download', 'start', "Size: {$file_size} bytes, Hash: {$file_hash}");
    
    // Stream the file
    $handle = fopen($firmware_path, 'rb');
    if ($handle) {
        while (!feof($handle)) {
            echo fread($handle, 8192);
            flush();
        }
        fclose($handle);
        
        logOTARequest($device_id, 'download', 'complete', "Size: {$file_size} bytes");
    } else {
        logOTARequest($device_id, 'download', 'error', 'Failed to open firmware file');
        http_response_code(500);
        echo json_encode(['error' => 'Failed to read firmware']);
    }
}

/**
 * Handle firmware info requests
 */
function getFirmwareInfo($firmware_path, $device_id) {
    if (!file_exists($firmware_path)) {
        logOTARequest($device_id, 'info', 'error', 'Firmware file not found');
        http_response_code(404);
        echo json_encode(['error' => 'Firmware not found']);
        return;
    }
    
    $file_size = filesize($firmware_path);
    $file_hash = md5_file($firmware_path);
    $file_time = filemtime($firmware_path);
    
    $info = [
        'status' => 'available',
        'size' => $file_size,
        'hash' => $file_hash,
        'timestamp' => $file_time,
        'date' => date('Y-m-d H:i:s', $file_time),
        'download_url' => 'http://iotlogic.in/ota/firmware.bin'
    ];
    
    logOTARequest($device_id, 'info', 'success', "Size: {$file_size} bytes");
    
    header('Content-Type: application/json');
    echo json_encode($info, JSON_PRETTY_PRINT);
}

// Main request handling
$device_id = getDeviceId();
$request_uri = $_SERVER['REQUEST_URI'];
$request_method = $_SERVER['REQUEST_METHOD'];

// Check device authorization
if (!isDeviceAuthorized($device_id)) {
    logOTARequest($device_id, 'access', 'denied', 'Device not authorized');
    http_response_code(403);
    echo json_encode(['error' => 'Device not authorized']);
    exit();
}

// Route requests
if ($request_method === 'GET') {
    $version = $_GET['version'] ?? 'latest';
    $firmware_path = getFirmwarePath($version);
    
    // Check if requesting firmware info or actual firmware
    if (strpos($request_uri, '/info') !== false || isset($_GET['info'])) {
        getFirmwareInfo($firmware_path, $device_id);
    } else {
        // Serve firmware file
        serveFirmware($firmware_path, $device_id);
    }
} else {
    logOTARequest($device_id, 'request', 'error', 'Invalid request method: ' . $request_method);
    http_response_code(405);
    echo json_encode(['error' => 'Method not allowed']);
}

?>
