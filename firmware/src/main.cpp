#include <Arduino.h>
#include <WiFi.h>
#include <cstddef>
#include "config_manager.h"
#include "web_portal.h"
#include "camera_manager.h"
#include "mqtt_client.h"
#include "hardware_utils.h"

AppConfig config;

void takeAndSendPicture(bool& imageOk, bool& commOk) {
    imageOk = commOk = false;
    
    // Bei Foto-Aktion: Grüne LED an
    HardwareUtils::setSuccessLED(true);
    Serial.println("[PIC] Starting picture sequence...");

    // (re-)init camera
    if (!CameraManager::begin(config)) {
        Serial.println("Camera init failed.");
        HardwareUtils::setSuccessLED(false);
        HardwareUtils::setErrorLED(true);
        commOk = imageOk = false;
        return;
    }

    // Fire flash, give it a moment
    CameraManager::setFlash(true);
    delay(200);

    // Capture into CameraManager’s static buffer
    if (!CameraManager::captureFrame()) {
        Serial.println("Image capture failed.");
        CameraManager::setFlash(false);
        HardwareUtils::setSuccessLED(false);
        HardwareUtils::setErrorLED(true);
        commOk = imageOk = false;
        return;
    }
    CameraManager::setFlash(false);
    imageOk = true;
    Serial.println("[PIC] Capture successful.");

    // Grab pointer+length
    size_t len;
    const uint8_t* data = CameraManager::imageData(len);
    Serial.printf("[PIC] Image size: %zu bytes\n", len);

    // Publish raw buffer directly
    Serial.println("[PIC] Publishing to MQTT...");
    commOk = MQTTClient::publishImage(data, len);
    if (commOk) {
        Serial.println("Image sent!");
        // Erfolg: Grün aus (Ruhezustand)
        HardwareUtils::setSuccessLED(false);
        HardwareUtils::setErrorLED(false);
    } else {
        HardwareUtils::setSuccessLED(false);
        HardwareUtils::setErrorLED(true);
        Serial.println("Image send failed.");
        // Kein Halt mehr, damit Retry möglich ist (DeepSleep oder manueller Trigger)
    }
}

void setup() {
    Serial.begin(115200);
    // Wait a moment for serial monitor to connect
    delay(1000);
    Serial.println("\n\n--- ESP32-CAM Booting ---");

    ConfigManager::begin();
    HardwareUtils::setupButtonsAndLEDs();
    
    // Boot-Prozess Start: Rote LED an
    HardwareUtils::setErrorLED(true);
    Serial.println("[BOOT] Loading configuration...");

    bool configNeeded = !ConfigManager::load(config) || HardwareUtils::isConfigButtonPressed();
    if (configNeeded) {
        Serial.println("[BOOT] No valid config or button pressed. Starting Web Portal.");
        WebPortal::begin(config, [](){ ESP.restart(); });
    } else {
        Serial.printf("[BOOT] Config loaded. Connecting to WiFi SSID: %s\n", config.wifi_ssid.c_str());
        WiFi.begin(config.wifi_ssid.c_str(), config.wifi_password.c_str());
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
            delay(500);
            Serial.print(".");
        }
        Serial.println();

        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[FATAL] WiFi connection failed! Going to sleep and retry later.");
            HardwareUtils::setErrorLED(true);
            delay(2000);
            // Nicht Config löschen! Sonst bleibt das Gerät hängen.
            // Versuche es in 60 Sekunden erneut.
            esp_sleep_enable_timer_wakeup(60ULL * 1000000ULL);
            esp_deep_sleep_start();
        }

        Serial.printf("[BOOT] WiFi connected. IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.println("[BOOT] Connecting to MQTT...");
        MQTTClient::begin(config);
        unsigned long t0 = millis();
        while (!MQTTClient::isConnected() && millis() - t0 < 5000) { MQTTClient::loop(); delay(50);} 
        
        if (MQTTClient::isConnected()) { 
            Serial.println("[BOOT] MQTT connected. Publishing discovery and availability.");
            MQTTClient::publishDiscovery(config); 
            MQTTClient::publishAvailability(true);
            // Abschluss Verbindung: Grün dazu, warten, dann beide aus
            HardwareUtils::setSuccessLED(true);
            delay(1000);
            HardwareUtils::setSuccessLED(false);
            HardwareUtils::setErrorLED(false);
        } else {
            Serial.println("[ERROR] MQTT connection failed.");
            // Rote LED bleibt an, aber wir machen weiter
        }

        Serial.println("[BOOT] Taking initial picture...");
        bool imageOk = false, commOk = false;
        takeAndSendPicture(imageOk, commOk);

        Serial.println("[MAIN] Entering 15s window for manual trigger or config mode...");

        // TEST-MODUS: Zeige aktuellen Status an
        Serial.printf("[TEST] Initial Button State (IO12): %s\n", HardwareUtils::isManualTriggerPressed() ? "PRESSED (LOW)" : "RELEASED (HIGH)");
        
        unsigned long windowStart = millis();
        while (millis() - windowStart < 15000) {
            if (HardwareUtils::isManualTriggerPressed()) {
                Serial.println("[TEST] Button signal detected (PRESSED)!");
                unsigned long pressStart = millis();
                
                // Warten auf Loslassen mit Timeout (Schutz gegen Hängenbleiben)
                while (HardwareUtils::isManualTriggerPressed()) {
                    delay(10);
                    // Timeout nach 3 Sekunden, falls der Button klemmt
                    if (millis() - pressStart > 3000) {
                        Serial.println("[TEST] TIMEOUT: Button seems stuck LOW. Breaking loop to continue.");
                        break; 
                    }
                }

                unsigned long pressMs = millis() - pressStart;
                Serial.printf("[TEST] Button RELEASED after %lu ms\n", pressMs);

                if (pressMs >= 1500) {
                    Serial.println("[ACTION] Long press detected. Entering config mode.");
                    WebPortal::begin(config, [](){ ESP.restart(); });
                    break;
                } else {
                    Serial.println("[ACTION] Short press detected. Taking picture.");
                    takeAndSendPicture(imageOk, commOk);
                    // Reset window timer after taking a picture
                    windowStart = millis();
                }
            }
            // WICHTIG: MQTT-Verbindung am Leben erhalten (KeepAlive senden),
            // sonst trennt der Broker die Verbindung wegen Timeout (k15)
            MQTTClient::loop();
            delay(10);
        }
        
        // After 15s window, decide whether to sleep or stay in web portal mode
        if (WebPortal::isActive()) {
            Serial.println("[MAIN] Web portal is active. Staying awake.");
            // Turn off status LEDs, we are in portal mode now
            HardwareUtils::setErrorLED(false);
            HardwareUtils::setSuccessLED(false);
        } else {
            Serial.println("[MAIN] 15s window timed out. Going to sleep for 1 minute.");
            if (MQTTClient::isConnected()) {
                MQTTClient::publishAvailability(false);
                delay(100); // Give MQTT time to send
            }
            
            // 1. Timer Wakeup (60 Sekunden)
            esp_sleep_enable_timer_wakeup(60ULL * 1000000ULL);

            // 2. Button Wakeup (Wenn IO12 auf GND gezogen wird)
            esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, 0);
            // 2. Button Wakeup (Wenn IO14 auf GND gezogen wird)
            esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, 0);

            Serial.println("[MAIN] Going to sleep (Wakeup: Timer or Button)...");
            Serial.flush(); // Warten bis Text gesendet wurde
            esp_deep_sleep_start();
        }
    }
}

void loop() {
    if (WebPortal::isActive()) {
        WebPortal::loop();
    }
    // In normal mode, MQTTClient::loop() is only called during connection attempts.
    // If we stay awake for other reasons, we might need to call it here.
    MQTTClient::loop();
}
