#include <Arduino.h>
#include "motion_detection.h"
#include "config.h"

// Basic frame-difference motion detection stub
static bool motion = false;

void motion_detection_init() {
  if (!ENABLE_MOTION_DETECTION) return;
  // Allocation would happen here
}

void motion_detection_loop() {
  if (!ENABLE_MOTION_DETECTION) return;
  // logic to update 'motion' variable would go here
}

bool motion_detected() {
  if (!ENABLE_MOTION_DETECTION) return false;
  return motion;
}
