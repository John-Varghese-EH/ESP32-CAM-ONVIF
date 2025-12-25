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

void setup() {
  Serial.begin(115200);
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
  } else {
    Serial.println("[INFO] WiFi not connected. RTSP and ONVIF services not started.");
    Serial.println("[INFO] Connect to AP network: " + wifiManager.getSSID());
    Serial.println("[INFO] Browse to: http://" + wifiManager.getLocalIP().toString());
  }
  
  // Initialize other services
  sd_recorder_init();
  motion_detection_init();
  
  Serial.println("[INFO] Setup complete. System running.");
}

void loop() {
  web_config_loop();
  rtsp_server_loop();
  onvif_server_loop();
  motion_detection_loop();
  sd_recorder_loop();
  serial_console_loop();
  // power management, stats, etc.
}
