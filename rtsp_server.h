#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "CRtspSession.h"
#include "MyStreamer.h"

extern WiFiServer rtspServer;
extern MyStreamer *streamer;

String getRTSPUrl();
void rtsp_server_start();
void rtsp_server_loop();
