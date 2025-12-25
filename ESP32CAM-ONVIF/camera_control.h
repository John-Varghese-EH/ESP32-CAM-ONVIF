#pragma once
bool camera_init();
void init_flash_led();
void set_flash_led(bool on);
void ptz_init();
void ptz_set_absolute(float x, float y);
