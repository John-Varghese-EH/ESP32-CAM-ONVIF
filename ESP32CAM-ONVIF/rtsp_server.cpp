#include "rtsp_server.h"
#include "config.h"

WiFiServer rtspServer(RTSP_PORT);
MyStreamer *streamer = nullptr;
CRtspSession *session = nullptr; // Keep track of the current session

String getRTSPUrl() {
  return "rtsp://" + WiFi.localIP().toString() + ":" + String(RTSP_PORT) + "/mjpeg/1";
}

void rtsp_server_start() {
  // The camera is already initialized in setup() via camera_init().
  // We just need to create the streamer.
  streamer = new MyStreamer();
  rtspServer.begin();
  Serial.println("[INFO] RTSP server started at " + getRTSPUrl());
}

void rtsp_server_loop() {
  // If we have an active session, handle it
  if (session) {
    session->handleRequests(0); // 0 timeout means non-blocking

    // Check if the client has disconnected
    if (session->m_stopped) {
      Serial.println("[INFO] RTSP client disconnected.");
      delete session;
      session = nullptr;
    }
  } else {
    // No active session, check for new clients
    WiFiClient client = rtspServer.available();
    if (client) {
      Serial.println("[INFO] RTSP client connected.");
      // Create a new session for the client.
      // The streamer is created once in rtsp_server_start().
      session = new CRtspSession(&client, streamer);
    }
  }
}
