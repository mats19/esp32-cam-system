#pragma once
#include <Arduino.h>

struct AppConfig {
    String wifi_ssid;
    String wifi_password;
    String mqtt_server;
    String mqtt_user;
    String mqtt_pass;
    String mqtt_topic;
    String camera_name;
    int    camera_resolution;
    int    camera_brightness;
    bool   flash_enabled;
    int    capture_start_hour;
    int    pictures_per_day;
    int    camera_contrast;
    int    camera_saturation;
    int    camera_wb_mode;
    int    camera_quality;
    bool   camera_hflip;
    bool   camera_vflip;
};

class ConfigManager {
public:
    static void begin();
    static bool load(AppConfig& config);
    static bool save(const AppConfig& config);
    static void reset();
};
