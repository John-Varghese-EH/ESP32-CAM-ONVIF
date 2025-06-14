// MyStreamer.cpp
#include "MyStreamer.h"

void MyStreamer::streamImage(uint32_t curMsec) {
    uint8_t *image = m_cam.getfb();
    uint32_t size = m_cam.getSize();
    if (image && size) {
        streamFrame(image, size, curMsec);
    }
}
