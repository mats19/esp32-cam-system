#include "camera_manager.h"
#include "config_manager.h"
#include <esp_camera.h>
#include <vector>
#include <Arduino.h>

#ifndef PWDN_GPIO_NUM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#endif

namespace CameraManager {

static std::vector<uint8_t> _imgBuf;
static bool _initialized = false;

bool begin(const AppConfig& config) {
    if (_initialized) return true; // Bereits initialisiert
    camera_config_t camCfg;
    camCfg.ledc_channel = LEDC_CHANNEL_0;
    camCfg.ledc_timer   = LEDC_TIMER_0;
    camCfg.pin_d0       = Y2_GPIO_NUM;
    camCfg.pin_d1       = Y3_GPIO_NUM;
    camCfg.pin_d2       = Y4_GPIO_NUM;
    camCfg.pin_d3       = Y5_GPIO_NUM;
    camCfg.pin_d4       = Y6_GPIO_NUM;
    camCfg.pin_d5       = Y7_GPIO_NUM;
    camCfg.pin_d6       = Y8_GPIO_NUM;
    camCfg.pin_d7       = Y9_GPIO_NUM;
    camCfg.pin_xclk     = XCLK_GPIO_NUM;
    camCfg.pin_pclk     = PCLK_GPIO_NUM;
    camCfg.pin_vsync    = VSYNC_GPIO_NUM;
    camCfg.pin_href     = HREF_GPIO_NUM;
    camCfg.pin_sccb_sda = SIOD_GPIO_NUM;
    camCfg.pin_sccb_scl = SIOC_GPIO_NUM;
    camCfg.pin_pwdn     = PWDN_GPIO_NUM;
    camCfg.pin_reset    = RESET_GPIO_NUM;

    camCfg.xclk_freq_hz = 20000000;
    camCfg.pixel_format = PIXFORMAT_JPEG;
    camCfg.frame_size   = (framesize_t)config.camera_resolution;
    camCfg.jpeg_quality = config.camera_quality > 0 ? config.camera_quality : 12;
    camCfg.fb_count     = 1;

    esp_err_t err = esp_camera_init(&camCfg);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed 0x%x\n", err);
        return false;
    }
    _initialized = true;
    sensor_t* s = esp_camera_sensor_get();
    if (s) {
        s->set_brightness(s, config.camera_brightness);
        s->set_contrast(s,   config.camera_contrast);
        s->set_saturation(s, config.camera_saturation);
        s->set_whitebal(s,   config.camera_wb_mode);
        s->set_hmirror(s,    config.camera_hflip);
        s->set_vflip(s,      config.camera_vflip);
    }
    return true;
}

bool captureFrame() {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb || fb->format != PIXFORMAT_JPEG) {
        if (fb) esp_camera_fb_return(fb);
        return false;
    }
    _imgBuf.assign(fb->buf, fb->buf + fb->len);
    esp_camera_fb_return(fb);
    return true;
}

const uint8_t* imageData(std::size_t& len) {
    len = _imgBuf.size();
    return _imgBuf.data();
}

void setFlash(bool on) { pinMode(4, OUTPUT); digitalWrite(4, on ? HIGH : LOW); }
void setBrightness(int b) { if (auto s = esp_camera_sensor_get()) s->set_brightness(s, b); }
void setContrast(int c)   { if (auto s = esp_camera_sensor_get()) s->set_contrast(s, c); }
void setSaturation(int s) { if (auto sensor = esp_camera_sensor_get()) sensor->set_saturation(sensor, s); }
void setWBMode(int m)     { if (auto s = esp_camera_sensor_get()) s->set_whitebal(s, m); }
void setQuality(int q)    { if (auto s = esp_camera_sensor_get()) s->set_quality(s, q); }
void setFrameSize(framesize_t fs) { if (auto s = esp_camera_sensor_get()) s->set_framesize(s, fs); }
void setFlip(bool h, bool v) { if (auto s = esp_camera_sensor_get()) { s->set_hmirror(s, h); s->set_vflip(s, v); } }

} // namespace CameraManager
