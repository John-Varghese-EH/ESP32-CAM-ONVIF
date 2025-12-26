#pragma once

// ==============================================================================
//   ESP32-CAM ONVIF/RTSP/HVR Configuration
// ==============================================================================
//   --- Project Attribution ---
//   GitHub: John-Varghese-EH
//   Project: ESP32-CAM ONVIF/RTSP/HVR
//   Version: 1.2
// ==============================================================================

// --- WiFi Settings ---
// Set these to your local network credentials for automatic connection
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

// --- AP Mode Settings ---
// Fallback access point if WiFi connection fails
#define AP_SSID         "ESP32CAM-ONVIF"
#define AP_PASSWORD     "esp32cam"      // Min 8 characters

// --- Security ---
// Credentials for the Web Configuration Interface
#define WEB_USER        "admin"
#define WEB_PASS        "esp123"

// --- Static IP Settings (Optional) ---
#define STATIC_IP_ENABLED   true       // Set to true to use Static IP
#define STATIC_IP_ADDR      192,168,0,150
#define STATIC_GATEWAY      192,168,0,1
#define STATIC_SUBNET       255,255,255,0
#define STATIC_DNS          8,8,8,8

// --- Server Ports ---
#define WEB_PORT        80              // Web configuration interface
#define RTSP_PORT       554             // RTSP Streaming port (standard: 554)
#define ONVIF_PORT      8000            // ONVIF Service port (standard: 80, 8000, or 8080)

// --- Flash LED Settings ---
// GPIO 4 is standard for ESP32-CAM Flash.
// WARNING: GPIO 4 is also SD Card Data 1. If FLASH_LED_ENABLED is true, SD card MUST use 1-bit mode.
// If you disable Flash, SD card can use 4-bit mode (faster).
#define FLASH_LED_ENABLED true          // Set to false to enable 4-bit SD mode
#define FLASH_LED_PIN     4
#define FLASH_LED_INVERT false          // false = High is ON
#define DEFAULT_AUTO_FLASH false        // Auto-flash disabled by default

// --- Status LED Settings ---
#define STATUS_LED_ENABLED true         // Set to false to disable Blue LED
#define STATUS_LED_PIN     33           // On-board Status LED (usually GPIO 33)
#define STATUS_LED_INVERT  true         // true = Low is ON (Standard for Blue LED)

// --- PTZ (Servo) Settings ---
// Optional: Connect servos for Pan/Tilt control
#define PTZ_ENABLED       false         // Set to true to enable servo control
#define SERVO_PAN_PIN     12            // GPIO for Pan Servo
#define SERVO_TILT_PIN    13            // GPIO for Tilt Servo

// --- Recording Settings ---
#define ENABLE_DAILY_RECORDING  false   // If true, records continuously (loop overwrite)
#define RECORD_SEGMENT_SEC      300     // 5 minutes per file
#define MAX_DISK_USAGE_PCT      90      // Auto-delete oldest files if disk usage > 90%
#define ENABLE_MOTION_DETECTION false    // Set false to disable motion detection to save CPU

// --- Device Information (ONVIF) ---
// These appear in your DVR/NVR during discovery
#define DEVICE_MANUFACTURER "John-Varghese-EH"
#define DEVICE_MODEL        "ESP32-CAM-ONVIF"
#define DEVICE_VERSION      "1.0"
#define DEVICE_HARDWARE_ID  "ESP32CAM-J0X"

// --- Time Settings ---
#define NTP_SERVER      "pool.ntp.org"
#define GMT_OFFSET_SEC  19800           // India (UTC+5:30) = 5.5 * 3600 = 19800
#define DAYLIGHT_OFFSET 0               // No DST in India

// --- Helper Functions ---
void printBanner();
void fatalError(const char* msg);
