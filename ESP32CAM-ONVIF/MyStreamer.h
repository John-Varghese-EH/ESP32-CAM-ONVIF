// MyStreamer.h
#pragma once
#include "OV2640.h"
#include "CStreamer.h"

class MyStreamer : public CStreamer {
public:
    MyStreamer(OV2640 &cam) : CStreamer(cam.getWidth(), cam.getHeight()), m_cam(cam) {}
    virtual ~MyStreamer() {}
    virtual void streamImage(uint32_t curMsec) override;
private:
    OV2640 &m_cam;
};
