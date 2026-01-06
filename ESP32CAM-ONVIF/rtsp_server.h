#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "board_config.h"
#include "CRtspSession.h"

// Conditionally include the appropriate streamer
#ifdef VIDEO_CODEC_H264
    #include "H264Streamer.h"
    extern H264Streamer *streamer;
#else
    #include "MyStreamer.h"
    extern MyStreamer *streamer;
#endif

extern WiFiServer rtspServer;

String getRTSPUrl();
void rtsp_server_start();
void rtsp_server_loop();

// Get current codec name for display
const char* getCodecName();
