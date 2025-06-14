#include <Arduino.h>
#include "motion_detection.h"
#include "esp_camera.h"

static bool motion = false;

void motion_detection_init() {
  // Initialize motion detection buffers, etc.
}

void motion_detection_loop() {
  // Compare frames, set motion flag
  // Very basic example (expand for real use)
  static unsigned long lastToggle = 0;
  if(millis() - lastToggle > 10000){
    motion = !motion;
    Serial.printf("[INFO] Motion detected: %s\n", motion ? "YES" : "NO");
    lastToggle = millis();
  }
}

bool motion_detected() {
  return motion;
}
