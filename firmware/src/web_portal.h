#pragma once
#include <Arduino.h>
#include "config_manager.h"

namespace WebPortal {
    void begin(AppConfig& config, void (*onConfigSaved)());
    void loop();
    bool isActive();
}
