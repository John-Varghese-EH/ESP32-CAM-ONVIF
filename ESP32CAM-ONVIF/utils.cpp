#include "utils.h"
#include <Arduino.h>

void printBanner() {
  Serial.println();
  Serial.println("==============================================");
  Serial.println("  ESP32CAM-ONVIF Professional Camera Firmware ");
  Serial.println("         Made with \xE2\x9D\xA4\xEF\xB8\x8F  by J0X           ");
  Serial.println("==============================================");
}

void fatalError(const char* msg) {
  Serial.println();
  Serial.print("[FATAL] "); Serial.println(msg);
  Serial.println("[FATAL] System halted.");
  while (1) delay(1000);
}
