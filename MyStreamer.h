// MyStreamer.h
#pragma once
#include "CStreamer.h"
#include "esp_camera.h"

class MyStreamer : public CStreamer {
public:
    MyStreamer();
    virtual ~MyStreamer() {}
    virtual void streamImage(uint32_t curMsec) override;
};
