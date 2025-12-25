#pragma once
#include <Arduino.h>

void sd_recorder_init();
void sd_recorder_loop();

// Helper to cleanup old files
void manage_storage();
