#pragma once

// ==============================================================================
//   ESP32-CAM ONVIF/RTSP/DVR Configuration
// ==============================================================================

// --- WiFi Settings ---
// Set these to your local network credentials for automatic connection
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

// --- AP Mode Settings ---
// Fallback access point if WiFi connection fails
#define AP_SSID         "ESP32CAM-ONVIF"
#define AP_PASSWORD     "esp32cam"      // Min 8 characters

// --- Server Ports ---
#define WEB_PORT        80              // Web configuration interface
#define RTSP_PORT       554             // RTSP Streaming port (standard: 554)
#define ONVIF_PORT      8000            // ONVIF Service port (standard: 80, 8000, or 8080)

// --- Security ---
// Credentials for the Web Configuration Interface
#define WEB_USER        "admin"
#define WEB_PASS        "esp123"

// --- Device Information (ONVIF) ---
// These appear in your DVR/NVR during discovery
#define DEVICE_MANUFACTURER "ESP32-CAM-J0X"
#define DEVICE_MODEL        "ONVIF-ESPCAM"
#define DEVICE_VERSION      "1.0"
#define DEVICE_SERIAL       "J0X-00001"
#define DEVICE_HARDWARE_ID  "ESP32CAM-J0X"

// --- Helper Functions ---
void printBanner();
void fatalError(const char* msg);
