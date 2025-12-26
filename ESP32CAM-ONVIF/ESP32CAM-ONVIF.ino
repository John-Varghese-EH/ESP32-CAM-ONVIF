/*
  ESP32-CAM Advanced ONVIF + RTSP + WebConfig + SD + Motion Detection

  Features:
  - ONVIF WS-Discovery responder + minimal SOAP device service
  - RTSP MJPEG streaming on port 554
  - Basic web server on port 80 for configuration placeholder
  - SD card initialization for recording (expand as needed)
  - Basic motion detection stub
  
  Made with ❤️ by J0X
*/


#include "camera_control.h"
#include "rtsp_server.h"
#include "onvif_server.h"
#include "web_config.h"
#include "sd_recorder.h"
#include "motion_detection.h"
#include "config.h"
#include "wifi_manager.h"
#include "serial_console.h"
#include "auto_flash.h"
#include "status_led.h"

void setup() {
  // Production speed: 115200. Debug output minimized via platformio.ini
  Serial.begin(115200);
  Serial.setDebugOutput(false); 
  
  Serial.println("\n\n--- ESP32-CAM STARTING ---");
  printBanner();
  
  // Initialize camera
  if (!camera_init()) fatalError("Camera init failed!");
  
  // Initialize WiFi - try stored credentials first, fallback to AP mode
  bool wifiConnected = wifiManager.begin();
  
  // Start web server (works in both AP and STA mode)
  web_config_start();
  
  // Only start these services if we're connected to a network
  if (wifiConnected) {
    rtsp_server_start();
    onvif_server_start();
    
    // Start Time Snyc
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET, NTP_SERVER);
    Serial.println("[INFO] NTP Time Sync started");
  } else {
    Serial.println("[INFO] WiFi not connected. RTSP/ONVIF disabled.");
    Serial.println("[INFO] Connect to AP: " + wifiManager.getSSID());
    Serial.println("[INFO] Browse to: http://" + wifiManager.getLocalIP().toString());
  }
  
  // Initialize other services (once)
  sd_recorder_init();
  motion_detection_init();
  auto_flash_init(); 
  status_led_init();
  status_led_flash(1); 
  
  if(wifiConnected) {
      status_led_connected(); 
  }
  else status_led_wifi_connecting();
  
  Serial.println("[INFO] System Ready.");
}

void loop() {
  // Critical Loops (Keep minimal blocking)
  rtsp_server_loop();   // Highest priority for streaming
  wifiManager.loop();   // Connectivity
  web_config_loop();    // Web UI
  onvif_server_loop();  // Discovery/SOAP
  
  // Background Tasks
  motion_detection_loop();
  sd_recorder_loop();
  serial_console_loop();
  auto_flash_loop();
  status_led_loop();
  
  // Optional: Power saving delay if NO clients connected? 
  // For RTSP low latency, we usually avoid delay, but a yield() helps watchdog.
  yield(); 
}
