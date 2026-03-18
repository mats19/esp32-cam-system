#include "mqtt_client.h"
#include <PubSubClient.h>
#include <WiFi.h>

static AppConfig   global_config;
static WiFiClient  wifiClient;
static PubSubClient mqttClient(wifiClient);

static unsigned long lastReconnectAttempt = 0;
static const unsigned long RECONNECT_INTERVAL_MS = 3000; // 3s

static String makeAvailTopic(const String& camName) {
    String s = camName;
    for (size_t i=0;i<s.length();++i) {
        char c = s[i];
        bool ok = (c>='a'&&c<='z')||(c>='0'&&c<='9')||(c=='-')||(c=='_');
        if (c>='A'&&c<='Z') { s.setCharAt(i, c+32); ok=true; }
        if (!ok) s.setCharAt(i, '_');
    }
    return String("esp32cam/") + s + "/availability";
}

static bool connectMqtt()
{
    String clientId = String("esp32cam-") + String((uint32_t)ESP.getEfuseMac(), HEX);
    String avail = makeAvailTopic(global_config.camera_name);

    bool ok = mqttClient.connect(
        clientId.c_str(),
        global_config.mqtt_user.length() ? global_config.mqtt_user.c_str() : nullptr,
        global_config.mqtt_pass.length() ? global_config.mqtt_pass.c_str() : nullptr,
        avail.c_str(),   // willTopic
        0,               // willQoS
        true,            // willRetain
        "offline"        // willMessage
    );
    if (ok) { mqttClient.publish(avail.c_str(), "online", true); }
    return ok;
}

void MQTTClient::begin(const AppConfig& config) {
    global_config = config;
    mqttClient.setServer(config.mqtt_server.c_str(), 1883);
    mqttClient.setKeepAlive(60); // Timeout auf 60s erhöhen (verhindert k15 Disconnects)
    
    // Puffer auf Max (65535) für PubSubClient setzen.
    // Achtung: PubSubClient nutzt uint16_t, 256000 würde überlaufen!
    // Mit PSRAM (via platformio.ini) ist das Allocieren sicher.
    if (!mqttClient.setBufferSize(65535)) {
        Serial.println("[MQTT] ERROR: Buffer allocation failed!");
    }
    lastReconnectAttempt = 0;
}

bool MQTTClient::publishImage(const uint8_t* data, size_t len) {
    if (!mqttClient.connected()) { MQTTClient::loop(); }
    if (!mqttClient.connected()) return false;

    Serial.printf("[MQTT] Sending %zu bytes to '%s'...\n", len, global_config.mqtt_topic.c_str());
    // WICHTIG: 'true' am Ende bedeutet Retain. Das Bild bleibt im Broker gespeichert.
    bool res = mqttClient.publish(global_config.mqtt_topic.c_str(), data, len, true);
    // Loop aufrufen, um das Senden im TCP-Stack anzustoßen
    mqttClient.loop();
    return res;
}

void MQTTClient::publishDiscovery(const AppConfig& config) {
    // 1. Topic-Name bereinigen (keine Leerzeichen für MQTT Topic)
    String safeName = config.camera_name;
    safeName.replace(" ", "_");
    safeName.toLowerCase();
    String discoveryTopic = "homeassistant/camera/" + safeName + "/config";

    // 2. Stabile Unique ID (MAC-basiert) und Device-Info hinzufügen
    String mac = String((uint32_t)ESP.getEfuseMac(), HEX);
    String uniqueId = "esp32cam_" + mac;

    String payload = "{";
    payload += "\"name\":\"" + config.camera_name + "\",";
    payload += "\"topic\":\"" + config.mqtt_topic + "\",";
    payload += "\"unique_id\":\"" + uniqueId + "\",";
    payload += "\"availability_topic\":\"" + makeAvailTopic(config.camera_name) + "\",";
    payload += "\"device\":{";
    payload += "\"identifiers\":[\"" + uniqueId + "\"],";
    payload += "\"name\":\"" + config.camera_name + "\",";
    payload += "\"model\":\"ESP32-CAM Meter\",\"manufacturer\":\"DIY\"}";
    payload += "}";
    mqttClient.publish(discoveryTopic.c_str(), payload.c_str(), true);
}

void MQTTClient::publishAvailability(bool online) {
    String avail = makeAvailTopic(global_config.camera_name);
    mqttClient.publish(avail.c_str(), online ? "online" : "offline", true);
}

void MQTTClient::loop() {
    if (!mqttClient.connected()) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt >= RECONNECT_INTERVAL_MS) {
            lastReconnectAttempt = now;
            (void)connectMqtt();
        }
    } else {
        mqttClient.loop();
    }
}

bool MQTTClient::isConnected() { return mqttClient.connected(); }
