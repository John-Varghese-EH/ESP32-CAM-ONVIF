#include "rtsp_server.h"
#include <WiFi.h>
#include "OV2640.h"
#include "CRtspSession.h"

WiFiServer rtspServer(554);
OV2640 cam;

String getRTSPUrl() {
  return "rtsp://" + WiFi.localIP().toString() + ":554/mjpeg/1";
}

void rtsp_server_start() {
  rtspServer.begin();
  cam.init(esp_camera_sensor_get());
  Serial.println("[INFO] RTSP server started at " + getRTSPUrl());
}

void rtsp_server_loop() {
  WiFiClient client = rtspServer.available();
  if (client) {
    CRtspSession session(client, cam);
    while (client.connected()) {
      session.handleRequests(0);
      session.broadcastCurrentFrame(0);
      delay(10);
    }
    client.stop();
  }
}
