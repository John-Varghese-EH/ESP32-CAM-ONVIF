#include <Arduino.h>
#include "camera_control.h"
#define CAM_TASK_STACK_SIZE 16384
#include "esp_camera.h"
#include "config.h"
#ifdef PTZ_ENABLED
#include <ESP32Servo.h>
Servo panServo;
Servo tiltServo;
#endif

bool camera_init() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sccb_sda = 26;
  config.pin_sccb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA; // 640x480 - Rock Solid Stability for NVRs
    config.jpeg_quality = 12;          // High quality (lower num)
    config.fb_count = 2;
    Serial.println(F("[INFO] PSRAM found. Using VGA (640x480) and 2 Frame Buffers"));
  } else {
    config.frame_size = FRAMESIZE_VGA; // Non-PSRAM cannot do HD well, fallback to SVGA
    config.jpeg_quality = 12;
    config.fb_count = 1;
    Serial.println(F("[WARN] No PSRAM. Using VGA and 1 Frame Buffer"));
  }
  
  // config.frame_size = FRAMESIZE_QVGA; // Reduce frame size to save memory // OLD FRAME SIZE
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[ERROR] Camera init failed: 0x%x\n", err);
    return false;
  }
  Serial.println("[INFO] Camera initialized.");
  
  if (FLASH_LED_ENABLED) {
    init_flash_led();
  }
  
  #if PTZ_ENABLED
    ptz_init();
  #endif

  return true;
}

void init_flash_led() {
    pinMode(FLASH_LED_PIN, OUTPUT);
    digitalWrite(FLASH_LED_PIN, FLASH_LED_INVERT ? HIGH : LOW); // Off by default
}

void set_flash_led(bool on) {
    if (!FLASH_LED_ENABLED) return;
    int state = on ? HIGH : LOW;
    if (FLASH_LED_INVERT) state = !state;
    digitalWrite(FLASH_LED_PIN, state);
}

#if PTZ_ENABLED
void ptz_init() {
    // Basic Servo init
    panServo.setPeriodHertz(50); 
    panServo.attach(SERVO_PAN_PIN, 500, 2400);
    tiltServo.setPeriodHertz(50);
    tiltServo.attach(SERVO_TILT_PIN, 500, 2400);
    
    // Center alignment
    panServo.write(90);
    tiltServo.write(90);
    Serial.println("[INFO] PTZ Servos initialized.");
}

// x, y are -1.0 to 1.0 (ONVIF standard usually) or 0.0 to 1.0
// We'll assume input 0.0 to 1.0 for absolute move
void ptz_set_absolute(float x, float y) {
    int panAngle = (int)(x * 180.0f);
    int tiltAngle = (int)(y * 180.0f);
    
    // Constrain
    if (panAngle < 0) panAngle = 0; if (panAngle > 180) panAngle = 180;
    if (tiltAngle < 0) tiltAngle = 0; if (tiltAngle > 180) tiltAngle = 180;
    
    panServo.write(panAngle);
    tiltServo.write(tiltAngle);
}

// Simple step move for continuous simulation
void ptz_move(float x_speed, float y_speed) {
    // Read current? Servo libs don't usually read back position easily.
    // We'll skip continuous move logic for now and stick to Absolute or Presets
    // since we don't track state well without a wrapper class.
}
#endif

