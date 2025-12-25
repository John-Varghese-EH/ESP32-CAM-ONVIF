#include <Arduino.h>
#include "auto_flash.h"
#include "config.h"
#include "camera_control.h"
#include "esp_camera.h"

static bool _auto_enabled = DEFAULT_AUTO_FLASH;
static unsigned long _last_check = 0;
static const unsigned long CHECK_INTERVAL = 2000; // Check every 2 seconds

void auto_flash_init() {
    _auto_enabled = DEFAULT_AUTO_FLASH;
}

void auto_flash_set_enabled(bool enabled) {
    _auto_enabled = enabled;
    if (!enabled) {
        // If disabling, turn off flash to be safe
        set_flash_led(false);
    }
}

bool auto_flash_is_enabled() {
    return _auto_enabled;
}

// Simple brightness calculation: average of a few skipped pixels to save time
static uint8_t get_brightness(camera_fb_t *fb) {
    if (!fb || fb->len == 0) return 0;
    
    // Only works reasonably well on JPEG if we assume DC components or just sample bytes, which is inaccurate for JPEG.
    // BUT, esp32-cam often returns JPEG.
    // If it's JPEG, we can't easily get brightness without decoding. 
    // HOWEVER, for simple "dark/light" detection, we might need to rely on the camera's AEC (Auto Exposure Control) values if accessible,
    // or just assume if it's nighttime, it's dark. 
    // Since we don't have a light sensor, and decoding JPEG is heavy...
    
    // Let's try a different approach: The camera sensor ESP32-CAM (OV2640) has internal AEC. 
    // We can read the exposure/gain values from the sensor registers.
    // If gain is high and exposure is maxed, it's dark.
    
    sensor_t * s = esp_camera_sensor_get();
    if (!s) return 0;
    
    // This is hardware specific for OV2640/OV3660
    // But standard driver usually exposes 'aec_value' or similar? 
    // No, standard driver abstracts it.
    
    // Fallback: If we can't easily get brightness, we might skip implementation or do a very rough check if we were in YUV/RGB mode.
    // But we are in JPEG mode.
    
    // ALTERNATIVE: Use the frame size? Darker images in JPEG *might* be smaller file size? Not reliable.
    
    // Let's rely on a valid check or user manual toggle if we can't get brightness.
    // But user asked for "auto flash".
    
    // Let's try to grab a low-res frame in GRAYSCALE just for this check? 
    // No, changing mode reboots/glitches stream.
    
    // What about "Mean" of bytes? In JPEG, it's entropy, not brightness.
    
    // OK, for now, let's implement a "Stub" that defaults to OFF,
    // or if we can access the Exposure value: s->status.aec_value
    // check s->status.aec_value or s->status.agc_value
    
    return s->status.aec_value; // Returns exposure value (0-1200 typically)
}

void auto_flash_loop() {
    if (!_auto_enabled) return;
    if (!FLASH_LED_ENABLED) return;
    
    if (millis() - _last_check < CHECK_INTERVAL) return;
    _last_check = millis();

    sensor_t * s = esp_camera_sensor_get();
    if (!s) return;
    
    // Thresholds need tuning.
    // High AEC value = Dark environment (camera trying to expose longer)
    // Low AEC value = Bright environment
    
    // Note: aec must be enabled.
    int exposure = s->status.aec_value; 
    int gain = s->status.agc_gain; // Correct member name for gain
    
    // Serial.printf("[DEBUG] Exp:%d Gain:%d\n", exposure, gain);
    
    // Simple logic: If exposure is hitting ceiling, it's dark.
    // Typ max exposure is around 1200ish (depends on clock)
    // Let's say if Expo > 1000 and Gain > 10 -> Dark
    
    bool is_dark = (exposure > 600) || (gain > 30); // Tuned loosely
    
    static bool is_led_on = false;
    
    if (is_dark && !is_led_on) {
        set_flash_led(true);
        is_led_on = true;
    } else if (!is_dark && is_led_on) {
        set_flash_led(false);
        is_led_on = false;
    }
}
