#include "web_portal.h"
#include "camera_manager.h"
#include "config_manager.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

AppConfig* portal_config = nullptr;
void (*configSavedCb)() = nullptr;
AsyncWebServer server(80);
DNSServer dnsServer;
bool portalActive = false;
bool shouldReboot = false;

namespace WebPortal {

void begin(AppConfig& config, void (*onConfigSaved)()) {
    portal_config = &config;
    configSavedCb = onConfigSaved;
    portalActive = true;

    WiFi.softAP("ESP32CAM-Setup");
    IPAddress myIP = WiFi.softAPIP();
    dnsServer.start(53, "*", myIP);

    CameraManager::begin(*portal_config);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = "<html><body><h2>ESP32CAM Setup</h2>"
                      "<form method='POST' action='/save'>"
                      "WiFi SSID: <input name='ssid' value='" + portal_config->wifi_ssid + "'><br>"
                      "WiFi Password: <input name='pwd' value='" + portal_config->wifi_password + "'><br>"
                      "MQTT Server: <input name='mqtt' value='" + portal_config->mqtt_server + "'><br>"
                      "MQTT User: <input name='user' value='" + portal_config->mqtt_user + "'><br>"
                      "MQTT Pass: <input name='pass' value='" + portal_config->mqtt_pass + "'><br>"
                      "MQTT Topic: <input name='topic' value='" + portal_config->mqtt_topic + "'><br>"
                      "Camera Name: <input name='camname' value='" + portal_config->camera_name + "'><br>"
                      "Resolution: <select name='camres'>";
        struct Opt { int v; const char* label; } opts[] = {
            {5,"VGA (640x480)"},{7,"SVGA (800x600)"},{8,"XGA (1024x768)"},
            {10,"SXGA (1280x1024)"},{11,"UXGA (1600x1200)"}
        };
        for (auto &o: opts) {
            html += String("<option value='") + o.v + "'" + (portal_config->camera_resolution==o.v?" selected":"") + ">" + o.label + "</option>";
        }
        html += "</select><br>"
                "Brightness (-2..2): <input name='cambr' type='number' min='-2' max='2' value='" + String(portal_config->camera_brightness) + "'><br>"
                "<input type='submit' value='Save & Reboot'></form>"
                "<hr><a href='/calibrate'>Calibrate Camera</a>"
                "</body></html>";
        request->send(200, "text/html", html);
    });

    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
        portal_config->wifi_ssid      = request->getParam("ssid", true)->value();
        portal_config->wifi_password  = request->getParam("pwd",  true)->value();
        portal_config->mqtt_server    = request->getParam("mqtt", true)->value();
        portal_config->mqtt_user      = request->getParam("user", true)->value();
        portal_config->mqtt_pass      = request->getParam("pass", true)->value();
        portal_config->mqtt_topic     = request->getParam("topic",true)->value();
        portal_config->camera_name    = request->getParam("camname", true)->value();
        if (request->hasParam("camres", true)) {
            portal_config->camera_resolution = request->getParam("camres", true)->value().toInt();
        }
        if (request->hasParam("cambr", true)) {
            portal_config->camera_brightness = request->getParam("cambr", true)->value().toInt();
        }
        ConfigManager::save(*portal_config);
        request->send(200, "text/html", "<h2>Saved! Rebooting...</h2>");
        shouldReboot = true;
    });

    server.on("/snapshot.jpg", HTTP_GET, [](AsyncWebServerRequest *request){
        if (!CameraManager::captureFrame()) {
            request->send(500, "text/plain", "Camera error");
            return;
        }
        std::size_t len;
        const uint8_t* data = CameraManager::imageData(len);
        
        // Fix für Kompilierungsfehler: Daten via Chunk-Callback senden
        AsyncWebServerResponse *response = request->beginResponse("image/jpeg", len, [data, len](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
            size_t copyLen = (len - index) < maxLen ? (len - index) : maxLen;
            memcpy(buffer, data + index, copyLen);
            return copyLen;
        });
        response->addHeader("Content-Disposition", "inline; filename=calibration.jpg");
        request->send(response);
    });

    server.on("/calibrate", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = "<html><body><h2>Camera Calibration</h2>"
                      "<img id='snap' src='/snapshot.jpg' width='320'><br>"
                      "<div>"
                      "Resolution: <select id='res'>"
                      "<option value='5'>VGA</option>"
                      "<option value='7'>SVGA</option>"
                      "<option value='8'>XGA</option>"
                      "<option value='10'>SXGA</option>"
                      "<option value='11'>UXGA</option>"
                      "</select>"
                      " Brightness: <input id='br' type='range' min='-2' max='2' step='1' value='" + String(portal_config->camera_brightness) + "'>"
                      " Quality: <input id='q' type='range' min='10' max='63' step='1' value='12'>"
                      " <button onclick='save()'>Save current</button>"
                      "</div>"
                      "<script>"
                      "function refresh(){ document.getElementById('snap').src='/snapshot.jpg?_='+Date.now(); }"
                      "setInterval(refresh,1000);"
                      "document.getElementById('res').value='" + String(portal_config->camera_resolution) + "';"
                      "document.getElementById('res').onchange=function(){ fetch('/set?res='+this.value); };"
                      "document.getElementById('br').oninput=function(){ fetch('/set?br='+this.value); };"
                      "document.getElementById('q').oninput=function(){ fetch('/set?q='+this.value); };"
                      "function save(){"
                      "  const r=document.getElementById('res').value;"
                      "  const b=document.getElementById('br').value;"
                      "  const q=document.getElementById('q').value;"
                      "  fetch('/set?res='+r+'&br='+b+'&q='+q+'&persist=1').then(()=>alert('Saved'));"
                      "}"
                      "</script>"
                      "<p>Adjust lens/focus; preview updates live to your settings.</p>"
                      "</body></html>";
        request->send(200, "text/html", html);
    });

    server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request){
        bool changed = false;
        if (request->hasParam("res")) {
            int v = request->getParam("res")->value().toInt();
            CameraManager::setFrameSize((framesize_t)v);
            portal_config->camera_resolution = v;
            changed = true;
        }
        if (request->hasParam("br")) {
            int v = request->getParam("br")->value().toInt();
            CameraManager::setBrightness(v);
            portal_config->camera_brightness = v;
            changed = true;
        }
        if (request->hasParam("q")) {
            int v = request->getParam("q")->value().toInt();
            CameraManager::setQuality(v);
            changed = true;
        }
        if (changed && request->hasParam("persist")) {
            ConfigManager::save(*portal_config);
        }
        request->send(200, "text/plain", "OK");
    });

    server.begin();
}

void loop() { 
    if (portalActive) dnsServer.processNextRequest(); 
    if (shouldReboot) {
        delay(2000); // Warten bis Response gesendet wurde
        ESP.restart();
    }
}
bool isActive() { return portalActive; }

} // namespace WebPortal
