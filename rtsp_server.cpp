#include "rtsp_server.h"

WiFiServer rtspServer(554);
OV2640 cam;
MyStreamer *streamer = nullptr;

String getRTSPUrl() {
  return "rtsp://" + WiFi.localIP().toString() + ":554/mjpeg/1";
}

void rtsp_server_start() {
  // Fill in your ESP32-CAM pin assignments
  camera_config_t config;
  config.pin_pwdn = -1;
  config.pin_reset = -1;
  config.pin_xclk = 4;
  config.pin_sscb_sda = 18;
  config.pin_sscb_scl = 23;
  config.pin_d7 = 36;
  config.pin_d6 = 37;
  config.pin_d5 = 38;
  config.pin_d4 = 39;
  config.pin_d3 = 35;
  config.pin_d2 = 14;
  config.pin_d1 = 13;
  config.pin_d0 = 34;
  config.pin_vsync = 5;
  config.pin_href = 27;
  config.pin_pclk = 25;
  config.xclk_freq_hz = 20000000;
  config.ledc_timer = LEDC_TIMER_0;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  cam.init(config);
  streamer = new MyStreamer(cam);
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
