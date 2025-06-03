#pragma once

// WiFi configuration
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

// WiFi connection and utility functions
bool wifi_connect();
void printBanner();
void fatalError(const char* msg);
