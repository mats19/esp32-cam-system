#include "config_manager.h"
#include <Preferences.h>
Preferences prefs;

#define CONFIG_NAMESPACE "esp32cam"

void ConfigManager::begin() {
    prefs.begin(CONFIG_NAMESPACE, false);
}

bool ConfigManager::load(AppConfig& config) {
    config.wifi_ssid            = prefs.getString("wifi_ssid", "");
    config.wifi_password        = prefs.getString("wifi_password", "");
    config.mqtt_server          = prefs.getString("mqtt_server", "");
    config.mqtt_user            = prefs.getString("mqtt_user", "");
    config.mqtt_pass            = prefs.getString("mqtt_pass", "");
    config.mqtt_topic           = prefs.getString("mqtt_topic", "esp32cam/meter/image");
    config.camera_name          = prefs.getString("camera_name", "Meter Camera");

    config.camera_resolution    = prefs.getInt("camera_resolution", 7); // SVGA
    config.camera_brightness    = prefs.getInt("camera_brightness", 0);
    config.flash_enabled        = prefs.getBool("flash_enabled", true);
    config.capture_start_hour   = prefs.getInt ("capture_start_hour", 12);
    config.pictures_per_day     = prefs.getInt ("pictures_per_day", 1);
    config.camera_contrast      = prefs.getInt ("camera_contrast", 0);
    config.camera_saturation    = prefs.getInt ("camera_saturation", 0);
    config.camera_wb_mode       = prefs.getInt ("camera_wb_mode", 0);
    config.camera_quality       = prefs.getInt ("camera_quality", 10);
    config.camera_hflip         = prefs.getBool("camera_hflip", false);
    config.camera_vflip         = prefs.getBool("camera_vflip", false);
    return !config.wifi_ssid.isEmpty();
}

bool ConfigManager::save(const AppConfig& config) {
    prefs.putString("wifi_ssid", config.wifi_ssid);
    prefs.putString("wifi_password", config.wifi_password);
    prefs.putString("mqtt_server", config.mqtt_server);
    prefs.putString("mqtt_user", config.mqtt_user);
    prefs.putString("mqtt_pass", config.mqtt_pass);
    prefs.putString("mqtt_topic", config.mqtt_topic);
    prefs.putString("camera_name", config.camera_name);
    prefs.putInt("camera_resolution", config.camera_resolution);
    prefs.putInt("camera_brightness", config.camera_brightness);
    prefs.putBool("flash_enabled",      config.flash_enabled);
    prefs.putInt ("capture_start_hour", config.capture_start_hour);
    prefs.putInt ("pictures_per_day",   config.pictures_per_day);
    prefs.putInt ("camera_contrast",    config.camera_contrast);
    prefs.putInt ("camera_saturation",  config.camera_saturation);
    prefs.putInt ("camera_wb_mode",     config.camera_wb_mode);
    prefs.putInt ("camera_quality",     config.camera_quality);
    prefs.putBool("camera_hflip",       config.camera_hflip);
    prefs.putBool("camera_vflip",       config.camera_vflip);
    return true;
}

void ConfigManager::reset() {
    prefs.clear();
}
