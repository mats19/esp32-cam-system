#pragma once
#include <Arduino.h>
#include <cstddef>
#include "config_manager.h"
#include <esp_camera.h>

namespace CameraManager {
    bool begin(const AppConfig& config);
    bool captureFrame();
    const uint8_t*  imageData(std::size_t& len);
    void setFlash(bool on);
    void setBrightness(int b);
    void setContrast(int c);
    void setSaturation(int s);
    void setWBMode(int mode);
    void setQuality(int q);
    void setFrameSize(framesize_t fs);
    void setFlip(bool h, bool v);
}
