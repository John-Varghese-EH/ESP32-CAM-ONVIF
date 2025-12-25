#include "rtsp_server.h"
#include "config.h"
#include "status_led.h"

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
    
    // Broadcast video frame (MJPEG)
    uint32_t now = millis();
    if(session && !session->m_stopped) { // Verify validity before streaming
        session->broadcastCurrentFrame(now);
    }

    // Check if the client has disconnected
    // Micro-RTSP session handles its own disconnection state often, but we need to check carefully
    // If handleRequests returns or sets a flag, we should check it.
    // Assuming m_stopped is the flag based on previous code.
    
    if (session->m_stopped) {
      Serial.println("[INFO] RTSP client disconnected.");
      delete session;
      session = nullptr;
    }
  } else {
    // No active session, check for new clients
    WiFiClient client = rtspServer.available();
    if (client) {
      // RTSP Crash Fix:
      // CRtspSession stores the SOCKET (WiFiClient*).
      // We MUST allocate it on host (heap) to survive this scope.
      // We also updated platglue-esp32.h to delete it on close.
      WiFiClient *clientPtr = new WiFiClient(client);
      
      // Ensure streamer is valid
      if (!streamer) {
          Serial.println("[ERROR] Streamer is NULL! Attempting re-init.");
          streamer = new MyStreamer();
      }
      
      if (streamer) {
          // Fix for TCP Transport Crash (0x0): 
          // Streamer needs the current client for RTP-over-TCP
          streamer->setClientSocket(clientPtr);
          
          session = new CRtspSession(clientPtr, streamer);
          // Flash LED twice to indicate successful RTSP connection (HVR Connected)
          status_led_flash(2); 
      } else {
          Serial.println("[FATAL] Streamer init failed. Closing client.");
          clientPtr->stop();
          delete clientPtr;
      } 
    }
  }
}
