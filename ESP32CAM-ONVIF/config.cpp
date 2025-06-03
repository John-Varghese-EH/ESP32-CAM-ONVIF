#include "config.h"
#include <WiFi.h>
#include <Arduino.h>

// Replace with your WiFi credentials
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

bool wifi_connect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[INFO] Connecting to WiFi");
  for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) {
    delay(250);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[INFO] WiFi connected: " + WiFi.localIP().toString());
    return true;
  } else {
    Serial.println("\n[ERROR] WiFi connect failed.");
    return false;
  }
}

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
