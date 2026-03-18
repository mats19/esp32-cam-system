#pragma once
#include <Arduino.h>
namespace HardwareUtils {
    void setupButtonsAndLEDs();
    bool isConfigButtonPressed();
    bool isManualTriggerPressed();
    void resetBoard();
    void setFlashLED(bool on);
    void setSuccessLED(bool on);
    void setErrorLED(bool on);
}
