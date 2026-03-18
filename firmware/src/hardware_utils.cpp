#include "hardware_utils.h"

#define CONFIG_BTN_PIN    14
#define SUCCESS_LED_PIN   15
#define ERROR_LED_PIN     13
#define FLASH_LED_PIN     4

void HardwareUtils::setupButtonsAndLEDs() {
    pinMode(CONFIG_BTN_PIN, INPUT_PULLUP);
    pinMode(SUCCESS_LED_PIN, OUTPUT); digitalWrite(SUCCESS_LED_PIN, LOW);
    pinMode(ERROR_LED_PIN, OUTPUT); digitalWrite(ERROR_LED_PIN, LOW);
    pinMode(FLASH_LED_PIN, OUTPUT); digitalWrite(FLASH_LED_PIN, LOW);
}

bool HardwareUtils::isConfigButtonPressed() {
    return digitalRead(CONFIG_BTN_PIN) == LOW;
}

bool HardwareUtils::isManualTriggerPressed() {
    return digitalRead(CONFIG_BTN_PIN) == LOW;
}

void HardwareUtils::resetBoard() {
    ESP.restart();
}

void HardwareUtils::setFlashLED(bool on) {
    digitalWrite(FLASH_LED_PIN, on ? HIGH : LOW);
}

void HardwareUtils::setSuccessLED(bool on) {
    digitalWrite(SUCCESS_LED_PIN, on ? HIGH : LOW);
}

void HardwareUtils::setErrorLED(bool on) {
    digitalWrite(ERROR_LED_PIN, on ? HIGH : LOW);
}
