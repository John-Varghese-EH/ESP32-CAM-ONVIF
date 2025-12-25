#include "sd_recorder.h"
#include "FS.h"
#include "SD_MMC.h"
#include "esp_camera.h" // Added for camera functions

#include "config.h"

void sd_recorder_init() {
  bool mountSuccess = false;
  
  if (FLASH_LED_ENABLED) {
    // 1-bit mode to free up GPIO 4 for Flash LED
    mountSuccess = SD_MMC.begin("/sdcard", true);
    if(mountSuccess) Serial.println("[INFO] SD Card mounted in 1-bit mode (Flash enabled).");
  } else {
    // 4-bit mode for higher speed (GPIO 4 used for Data 1)
    mountSuccess = SD_MMC.begin();
    if(mountSuccess) Serial.println("[INFO] SD Card mounted in 4-bit mode (Flash disabled).");
  }

  if(!mountSuccess){
    Serial.println("[WARN] SD Card Mount Failed");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("[WARN] No SD Card attached");
    return;
  }
  Serial.println("[INFO] SD Card initialized");
}

// --- Recording Globals ---
unsigned long _lastRecordFrame = 0;
unsigned long _currentSegmentStart = 0;
File _recordFile;
bool _isRecording = false;
int _segmentCounter = 0;

void manage_storage() {
    float total = SD_MMC.totalBytes();
    float used = SD_MMC.usedBytes();
    float pct = (used / total) * 100.0;
    
    if (pct > MAX_DISK_USAGE_PCT) {
        Serial.printf("[WARN] Disk Usage %.1f%% > %d%%. Cleaning up...\n", pct, MAX_DISK_USAGE_PCT);
        
        File root = SD_MMC.open("/recordings");
        if (!root) return;
        
        // Simple strategy: Delete oldest file
        // Since we name them by timestamp or counter, we could sort.
        // For simplicity, we assume alphabetical is roughly chronological if named properly.
        // Actually, let's just delete the first file we find that is a recording.
        
        File file = root.openNextFile();
        if (file) {
            String path = String("/recordings/") + file.name();
            file.close(); // Close before delete
            Serial.println("[INFO] Deleting old recording: " + path);
            SD_MMC.remove(path);
        }
    }
}

void start_new_segment() {
     if (_recordFile) {
        _recordFile.close();
    }
    
    // Ensure directory exists
    if (!SD_MMC.exists("/recordings")) {
        SD_MMC.mkdir("/recordings");
    }
    
    // Manage storage before creating new file
    manage_storage();
    
    String filename = "/recordings/rec_" + String(millis()) + ".mjpeg";
    _recordFile = SD_MMC.open(filename, FILE_WRITE);
    
    if (_recordFile) {
        Serial.println("[INFO] Started recording segment: " + filename);
        _currentSegmentStart = millis();
        _isRecording = true;
    } else {
        Serial.println("[ERROR] Failed to open recording file");
        _isRecording = false;
    }
}

void sd_recorder_loop() {
    if (!ENABLE_DAILY_RECORDING) return;
    if (FLASH_LED_ENABLED && !wifiManager.isInAPMode()) { 
        // Optional optimization: disable recording during heavy WiFi use if causing instability?
        // But user wants "even if connection fails".
    }

    unsigned long now = millis();
    
    // 1. Check segment time
    if (_isRecording && (now - _currentSegmentStart > (RECORD_SEGMENT_SEC * 1000))) {
        start_new_segment();
    }
    
    // 2. Init if needed
    if (!_isRecording) {
        start_new_segment();
        if (!_isRecording) return; // Still failed
    }

    // 3. Record Frame (Limit FPS to something reasonable, e.g., 5 FPS for background recording)
    if (now - _lastRecordFrame > 200) { // 5 FPS
        camera_fb_t * fb = esp_camera_fb_get();
        if (!fb) return;
        
        // Write MJPEG frame header + body
        // MJPEG boundary
        // We write raw JPEGs back to back. Some players need boundary headers, most VLC-like just play concatenated JPEGs.
        // A generic MJPEG stream usually just needs the JPEG bytes.
        
        // OPTIMIZATION: Check available space in write buffer?
        if (_recordFile.write(fb->buf, fb->len) != fb->len) {
            Serial.println("[ERROR] Write failed. Disk full?");
            _recordFile.close();
            _isRecording = false;
        } else {
            // Include flush occasionally? SD_MMC is buffered.
        }
        
        esp_camera_fb_return(fb);
        _lastRecordFrame = now;
    }
}
