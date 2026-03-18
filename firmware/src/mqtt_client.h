#pragma once
#include <Arduino.h>
#include "config_manager.h"

namespace MQTTClient {
    void begin(const AppConfig& config);
    bool publishImage(const uint8_t* data, size_t len);
    void publishDiscovery(const AppConfig& config);
    void publishAvailability(bool online);
    void loop();
    bool isConnected();
}
