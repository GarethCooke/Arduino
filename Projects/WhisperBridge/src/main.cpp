#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <Preferences.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "config.h"
#include "ble.h"
#include "mqtt.h"

static Preferences     s_prefs;
static AsyncWebServer  s_server(80);
static DNSServer       s_dns;
static bool            s_apMode = false;

// Returns "rrggbb" style suffix from last 3 MAC bytes
static String getDeviceId() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char id[8];
    snprintf(id, sizeof(id), "%02x%02x%02x", mac[3], mac[4], mac[5]);
    return String(id);
}

// ── WiFi ──────────────────────────────────────────────────────────────────────

static bool connectWifi() {
    s_prefs.begin("whisper", /*readOnly=*/true);
    String ssid = s_prefs.getString("ssid", "");
    String pass = s_prefs.getString("pass", "");
    s_prefs.end();

    if (ssid.isEmpty()) return false;

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(WIFI_HOSTNAME);
    WiFi.begin(ssid.c_str(), pass.c_str());

    Serial.printf("[WiFi] Connecting to %s", ssid.c_str());
    unsigned long t = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t < 12000) {
        delay(500);
        Serial.print('.');
    }
    Serial.println();
    return WiFi.status() == WL_CONNECTED;
}

// ── AP / captive-portal mode ──────────────────────────────────────────────────

static void startAP() {
    s_apMode = true;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
    s_dns.start(53, "*", WiFi.softAPIP());
    Serial.printf("[WiFi] AP started: %s  IP: %s\n",
                  WIFI_AP_SSID, WiFi.softAPIP().toString().c_str());

    // Redirect everything to /setup
    s_server.onNotFound([](AsyncWebServerRequest* req) {
        req->redirect("http://" + WiFi.softAPIP().toString() + "/setup.html");
    });

    // Save credentials and reboot
    s_server.on("/api/networkset", HTTP_POST,
        [](AsyncWebServerRequest* req) {},  // body handler does the work
        nullptr,
        [](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t) {
            JsonDocument doc;
            if (deserializeJson(doc, data, len) != DeserializationError::Ok ||
                !doc["ssid"].is<const char*>()) {
                req->send(400, "application/json", "{\"error\":\"bad json\"}");
                return;
            }
            s_prefs.begin("whisper", false);
            s_prefs.putString("ssid", doc["ssid"].as<String>());
            s_prefs.putString("pass", doc["password"] | "");
            s_prefs.end();
            req->send(200, "application/json", "{\"ok\":true}");
            delay(500);
            ESP.restart();
        }
    );

    s_server.serveStatic("/", LittleFS, "/");
    s_server.begin();
}

// ── Station mode ──────────────────────────────────────────────────────────────

static void startStation(const String& deviceId) {
    Serial.printf("[WiFi] Connected — IP: %s\n", WiFi.localIP().toString().c_str());

    MDNS.begin(WIFI_HOSTNAME);
    MDNS.addService("http", "tcp", 80);

    ArduinoOTA.setHostname(WIFI_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.onStart([]()  { Serial.println("[OTA] Start"); });
    ArduinoOTA.onEnd([]()    { Serial.println("[OTA] Done"); });
    ArduinoOTA.onError([](ota_error_t e) { Serial.printf("[OTA] Error %u\n", e); });
    ArduinoOTA.begin();

    // Status JSON
    s_server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* req) {
        JsonDocument doc;
        doc["running"]     = Ble.isRunning();
        doc["ble_ok"]      = Ble.lastSuccess();
        doc["ip"]          = WiFi.localIP().toString();
        doc["rssi"]        = WiFi.RSSI();
        char buf[256];
        serializeJson(doc, buf, sizeof(buf));
        req->send(200, "application/json", buf);
    });

    // Trigger boost via web UI
    s_server.on("/api/boost", HTTP_POST, [](AsyncWebServerRequest* req) {
        if (Ble.isRunning()) {
            req->send(409, "application/json", "{\"error\":\"already running\"}");
            return;
        }
        Ble.trigger();
        req->send(200, "application/json", "{\"ok\":true}");
    });

    s_server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    s_server.begin();

    Mqtt.setup(deviceId.c_str());
}

// ── Arduino entry points ──────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(500);

    if (!LittleFS.begin(true)) {
        Serial.println("[FS] LittleFS mount failed");
    }

    String deviceId = getDeviceId();
    Serial.printf("\n[Main] WhisperBridge  id=%s\n", deviceId.c_str());

    Ble.setup();

    if (connectWifi()) {
        startStation(deviceId);
    } else {
        startAP();
    }
}

void loop() {
    if (s_apMode) {
        s_dns.processNextRequest();
        return;
    }

    ArduinoOTA.handle();
    Mqtt.loop();

    // Once the BLE task finishes, publish the OFF state
    static bool s_wasRunning = false;
    const bool running = Ble.isRunning();
    if (s_wasRunning && !running) {
        Mqtt.publishState(false);
    }
    s_wasRunning = running;
}
