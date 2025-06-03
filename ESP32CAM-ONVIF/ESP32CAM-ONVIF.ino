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
#include "utils.h"

void setup() {
  Serial.begin(115200);
  printBanner();
  if (!camera_init()) fatalError("Camera init failed!");
  if (!wifi_connect()) fatalError("WiFi connect failed!");
  web_config_start();
  rtsp_server_start();
  onvif_server_start();
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
  // power management, stats, etc.
}
