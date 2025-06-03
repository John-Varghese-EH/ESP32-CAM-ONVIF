#include "web_config.h"
#include <WebServer.h>
#include <WiFi.h>
#include "rtsp_server.h"

WebServer webConfigServer(80);

void handle_root() {
  String html = "<html><head><title>J0X ESP32-CAM Config</title></head><body>"
                "<h1>ESP32-CAM Configuration</h1>"
                "<p>WiFi IP: " + WiFi.localIP().toString() + "</p>"
                "<p>RTSP Stream: " + getRTSPUrl() + "</p>"
                "<p>ONVIF Service: http://" + WiFi.localIP().toString() + ":8000/onvif/device_service</p>"
                "<footer style='margin-top:30px;color:#888;'>Made with &#10084;&#65039; by J0X</footer>"
                "</body></html>";
  webConfigServer.send(200, "text/html", html);
}

void web_config_start() {
  webConfigServer.on("/", HTTP_GET, handle_root);
  webConfigServer.begin();
  Serial.println("[INFO] Web config server started.");
}

void web_config_loop() {
  webConfigServer.handleClient();
}
