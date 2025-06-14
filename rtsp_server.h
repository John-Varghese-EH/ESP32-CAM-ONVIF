#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "OV2640.h"
#include "CRtspSession.h"
#include "MyStreamer.h"

extern WiFiServer rtspServer;
extern OV2640 cam;
extern MyStreamer *streamer;

String getRTSPUrl();
void rtsp_server_start();
void rtsp_server_loop();
