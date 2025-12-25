#include "wifi_manager.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "config.h"

// Global instance
WiFiManager wifiManager;

WiFiManager::WiFiManager() : _apMode(false), _scannedNetworksCount(0), _scannedNetworks(nullptr) {
}

bool WiFiManager::begin() {
  // First try to connect using stored credentials
  if (connectToStoredNetwork()) {
    return true;
  }

  // If stored credentials failed or missing, try hardcoded config.h credentials
  String configSSID = WIFI_SSID;
  if(configSSID != "YOUR_WIFI_SSID" && configSSID.length() > 0) {
      Serial.println("[INFO] Trying config.h credentials...");
      if(connectToNetwork(WIFI_SSID, WIFI_PASSWORD)) {
          return true;
      }
  }
  
  // If stored credentials don't work, start AP mode
  startAPMode();
  return false;
}

bool WiFiManager::connectToStoredNetwork() {
  WiFiCredentials creds = loadCredentials();
  
  if (creds.ssid.length() == 0) {
    Serial.println("[INFO] No stored WiFi credentials found");
    return false;
  }
  
  return connectToNetwork(creds.ssid, creds.password);
}

bool WiFiManager::connectToNetwork(const String& ssid, const String& password) {
  _apMode = false;
  WiFi.mode(WIFI_STA);
  
  if (STATIC_IP_ENABLED) {
    IPAddress ip(STATIC_IP_ADDR);
    IPAddress gateway(STATIC_GATEWAY);
    IPAddress subnet(STATIC_SUBNET);
    IPAddress dns(STATIC_DNS);
    
    if (WiFi.config(ip, gateway, subnet, dns)) {
      Serial.println("[INFO] Static IP Configured: " + ip.toString());
    } else {
      Serial.println("[WARN] Static IP Configuration Failed. Falling back to DHCP.");
    }
  }
  
  WiFi.begin(ssid.c_str(), password.c_str());
  
  Serial.print("[INFO] Connecting to WiFi: ");
  Serial.println(ssid);
  
  // Wait for connection
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(250);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[INFO] WiFi connected: " + WiFi.localIP().toString());
    return true;
  } else {
    Serial.println("\n[ERROR] WiFi connect failed.");
    return false;
  }
}

void WiFiManager::startAPMode() {
  Serial.println("[INFO] Starting AP mode");
  _apMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("[INFO] AP IP address: ");
  Serial.println(IP);
}

bool WiFiManager::isInAPMode() {
  return _apMode;
}

int WiFiManager::scanNetworks() {
  // Free previous scan results
  if (_scannedNetworks != nullptr) {
    delete[] _scannedNetworks;
    _scannedNetworks = nullptr;
  }
  
  Serial.println("[INFO] Scanning for networks...");
  
  // Scan for networks
  int found = WiFi.scanNetworks();
  _scannedNetworksCount = found;
  
  if (found > 0) {
    // Allocate memory for network data
    _scannedNetworks = new WiFiNetwork[found];
    
    // Store network details
    for (int i = 0; i < found; i++) {
      _scannedNetworks[i].ssid = WiFi.SSID(i);
      _scannedNetworks[i].rssi = WiFi.RSSI(i);
      _scannedNetworks[i].encType = WiFi.encryptionType(i);
    }
  }
  
  Serial.printf("[INFO] Found %d networks\n", found);
  return found;
}

WiFiNetwork* WiFiManager::getScannedNetworks() {
  return _scannedNetworks;
}

int WiFiManager::getScannedNetworksCount() {
  return _scannedNetworksCount;
}

bool WiFiManager::saveCredentials(const String& ssid, const String& password) {
  StaticJsonDocument<256> doc;
  doc["ssid"] = ssid;
  doc["password"] = password;
  
  File file = SPIFFS.open(_credentialsFile, "w");
  if (!file) {
    Serial.println("[ERROR] Failed to open credentials file for writing");
    return false;
  }
  
  if (serializeJson(doc, file) == 0) {
    Serial.println("[ERROR] Failed to write credentials to file");
    file.close();
    return false;
  }
  
  file.close();
  Serial.println("[INFO] WiFi credentials saved");
  return true;
}

WiFiCredentials WiFiManager::loadCredentials() {
  WiFiCredentials creds;
  
  if (!SPIFFS.exists(_credentialsFile)) {
    Serial.println("[INFO] Credentials file does not exist");
    return creds; // Return empty credentials
  }
  
  File file = SPIFFS.open(_credentialsFile, "r");
  if (!file) {
    Serial.println("[ERROR] Failed to open credentials file for reading");
    return creds;
  }
  
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.print("[ERROR] Failed to parse credentials JSON: ");
    Serial.println(error.c_str());
    return creds;
  }
  
  creds.ssid = doc["ssid"].as<String>();
  creds.password = doc["password"].as<String>();
  
  Serial.println("[INFO] WiFi credentials loaded");
  return creds;
}

IPAddress WiFiManager::getLocalIP() {
  if (_apMode) {
    return WiFi.softAPIP();
  } else {
    return WiFi.localIP();
  }
}

String WiFiManager::getSSID() {
  if (_apMode) {
    return String(AP_SSID);
  } else {
    return WiFi.SSID();
  }
}