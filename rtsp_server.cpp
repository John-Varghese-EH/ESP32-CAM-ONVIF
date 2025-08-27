#include "rtsp_server.h"

WiFiServer rtspServer(554);
MyStreamer *streamer = nullptr;

String getRTSPUrl() {
  return "rtsp://" + WiFi.localIP().toString() + ":554/mjpeg/1";
}

void rtsp_server_start() {
  // The camera is already initialized in setup() via camera_init().
  // We just need to create the streamer.
  streamer = new MyStreamer();
  rtspServer.begin();
  Serial.println("[INFO] RTSP server started at " + getRTSPUrl());
}

void rtsp_server_loop() {
  WiFiClient client = rtspServer.available();
  if (client) {
    // Micro-RTSP expects a SOCKET (int) and a CStreamer*
    // On ESP32, WiFiClient.fd() is not available, so we use a hack:
    // Pass the client object as a void* and cast it back in the library.
    // This requires a custom build of Micro-RTSP or using the ESP32-compatible fork.
    // For simplicity, this example assumes you have modified Micro-RTSP to accept WiFiClient*.
    // If not, use the standard Micro-RTSP example and adapt as needed.
    // For now, this is a placeholder:
    // CRtspSession session(client, streamer); // Won't work out of the box!
    // Instead, use the following workaround (requires library modification):
    // CRtspSession session((void*)&client, streamer);
    // Or use the standard Micro-RTSP example code.

    // IMPORTANT: The standard Micro-RTSP library does not support WiFiClient directly.
    // You must either:
    // 1. Use the standard Micro-RTSP example with a custom streamer, or
    // 2. Modify the library to accept WiFiClient* (advanced).

    // For now, here is a placeholder. See notes below for a real solution.
    Serial.println("Client connected, but RTSP session handling is not implemented.");
    while (client.connected()) {
      // Handle client here if you modify the library
      delay(10);
    }
    client.stop();
  }
}
