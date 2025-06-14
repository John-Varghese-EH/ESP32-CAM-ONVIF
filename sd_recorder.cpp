#include "sd_recorder.h"
#include "FS.h"
#include "SD_MMC.h"

void sd_recorder_init() {
  if(!SD_MMC.begin()){
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

void sd_recorder_loop() {
  // Implement motion-triggered or continuous recording
  // Write JPEG frames or MJPEG to SD card
}
