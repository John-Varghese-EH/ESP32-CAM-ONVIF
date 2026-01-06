#pragma once
#include <Arduino.h>

void sd_recorder_init();
void sd_recorder_loop();

// Helper to cleanup old files
void manage_storage();

// Manual Recording Control
void sd_recorder_start_manual();
void sd_recorder_stop_manual();
bool sd_recorder_is_recording();
bool sd_recorder_is_mounted();
