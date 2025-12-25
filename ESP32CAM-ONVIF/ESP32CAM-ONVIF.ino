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
#include "serial_console.h"
#include "auto_flash.h"
#include "status_led.h"

void setup() {
  Serial.begin(112500);
  Serial.setDebugOutput(true); // Enable detailed ESP logs
  delay(3000); // Wait for Serial Monitor
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
  } else {
    Serial.println("[INFO] WiFi not connected. RTSP and ONVIF services not started.");
    Serial.println("[INFO] Connect to AP network: " + wifiManager.getSSID());
    Serial.println("[INFO] Browse to: http://" + wifiManager.getLocalIP().toString());
  }
  
  // Initialize other services
  sd_recorder_init();
  motion_detection_init();
  motion_detection_init();
  auto_flash_init(); // Init auto flash before logic starts
  status_led_init();
  status_led_flash(1); // One flash -> Init started
  
  // Initial Status: Off or Connecting?
  // WiFi manager will start connecting soon, but we are already past wifiManager.begin()
  // If wifiConnected is true, set Solid -> Now OFF + Flash
  if(wifiConnected) {
      status_led_connected(); // Will flash twice and turn off
  }
  else status_led_wifi_connecting(); // Connection loop will make it blink if needed
  
  Serial.println("[INFO] Setup complete. System running.");
}

void loop() {
  wifiManager.loop(); // Check connection
  web_config_loop();
  rtsp_server_loop();
  onvif_server_loop();
  motion_detection_loop();
  sd_recorder_loop();
  serial_console_loop();
  sd_recorder_loop();
  serial_console_loop();
  auto_flash_loop();
  status_led_loop();
  // power management, stats, etc.
}
