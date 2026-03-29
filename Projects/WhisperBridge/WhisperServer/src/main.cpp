#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <Preferences.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <algorithm>
#include <map>
#include <string>
#include "config.h"
#include "ble.h"
#include "mqtt.h"

static Preferences    s_prefs;
static AsyncWebServer s_server(80);
static DNSServer      s_dns;
static bool           s_apMode   = false;
static String         s_deviceId;  // set once in setup()

// ── WiFi scan (AP mode only) ───────────────────────────────────────────────────

static int  s_scanCount = WIFI_SCAN_FAILED;
static bool s_scanReady = false;

static void pollScan() {
    int n = WiFi.scanComplete();
    if (n == WIFI_SCAN_RUNNING) return;
    if (n >= 0) {
        s_scanCount = n;
        s_scanReady = true;
    } else {
        s_scanReady = false;
        WiFi.scanNetworks(/*async=*/true);
    }
}

// ── Helpers ───────────────────────────────────────────────────────────────────

static String getDeviceId() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char id[8];
    snprintf(id, sizeof(id), "%02x%02x%02x", mac[3], mac[4], mac[5]);
    return String(id);
}

// ── Common endpoints (registered in both AP and station mode) ─────────────────

static void registerCommonEndpoints() {
    s_server.on("/api/deviceinfo", HTTP_GET, [](AsyncWebServerRequest* req) {
        String json = "{\"id\":\"" + s_deviceId + "\",\"url\":\"http://" WIFI_HOSTNAME ".local\"}";
        req->send(200, "application/json", json);
    });
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
    WiFi.softAPConfig(IPAddress(10,0,0,1), IPAddress(10,0,0,1), IPAddress(255,255,255,0));
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
    s_dns.start(53, "*", IPAddress(10,0,0,1));
    Serial.printf("[WiFi] AP started: %s  IP: 10.0.0.1\n", WIFI_AP_SSID);

    registerCommonEndpoints();

    // Available networks (async scan, polled from loop)
    s_server.on("/api/networkdata", HTTP_GET, [](AsyncWebServerRequest* req) {
        if (!s_scanReady) {
            req->send(200, "application/json", "{\"scanning\":true,\"networks\":[]}");
            return;
        }

        // Deduplicate by SSID, keep strongest signal per network
        struct NetInfo { int rssi; bool secure; };
        std::map<std::string, NetInfo> best;
        for (int i = 0; i < s_scanCount; i++) {
            std::string ssid = WiFi.SSID(i).c_str();
            if (ssid.empty()) continue;
            int  rssi   = WiFi.RSSI(i);
            bool secure = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
            auto it = best.find(ssid);
            if (it == best.end() || rssi > it->second.rssi)
                best[ssid] = {rssi, secure};
        }

        // Sort strongest first
        std::vector<std::pair<std::string, NetInfo>> sorted(best.begin(), best.end());
        std::sort(sorted.begin(), sorted.end(),
                  [](const auto& a, const auto& b) { return a.second.rssi > b.second.rssi; });

        JsonDocument doc;
        doc["scanning"] = false;
        JsonArray nets = doc["networks"].to<JsonArray>();
        for (const auto& [ssid, info] : sorted) {
            JsonObject n = nets.add<JsonObject>();
            n["ssid"]   = ssid;
            n["rssi"]   = info.rssi;
            n["secure"] = info.secure;
        }

        // Kick off a fresh scan for the next request
        s_scanReady = false;
        WiFi.scanDelete();
        WiFi.scanNetworks(/*async=*/true);

        String output;
        serializeJson(doc, output);
        req->send(200, "application/json", output);
    });

    // Save credentials and reboot
    s_server.on("/api/networkset", HTTP_POST,
        [](AsyncWebServerRequest* req) {},
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

    s_server.onNotFound([](AsyncWebServerRequest* req) {
        req->redirect("http://10.0.0.1/setup.html");
    });

    s_server.serveStatic("/", LittleFS, "/");
    s_server.begin();

    WiFi.scanNetworks(/*async=*/true);  // kick off first scan
}

// ── Station mode ──────────────────────────────────────────────────────────────

static void startStation() {
    Serial.printf("[WiFi] Connected — IP: %s\n", WiFi.localIP().toString().c_str());

    MDNS.begin(WIFI_HOSTNAME);
    MDNS.addService("http", "tcp", 80);

    ArduinoOTA.setHostname(WIFI_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.onStart([]()  { Serial.println("[OTA] Start"); });
    ArduinoOTA.onEnd([]()    { Serial.println("[OTA] Done"); });
    ArduinoOTA.onError([](ota_error_t e) { Serial.printf("[OTA] Error %u\n", e); });
    ArduinoOTA.begin();

    registerCommonEndpoints();

    s_server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* req) {
        JsonDocument doc;
        doc["running"] = Ble.isRunning();
        doc["ble_ok"]  = Ble.lastSuccess();
        doc["ip"]      = WiFi.localIP().toString();
        doc["rssi"]    = WiFi.RSSI();
        char buf[256];
        serializeJson(doc, buf, sizeof(buf));
        req->send(200, "application/json", buf);
    });

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

    Mqtt.setup(s_deviceId.c_str());
}

// ── Arduino entry points ──────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(500);

    if (!LittleFS.begin(true)) {
        Serial.println("[FS] LittleFS mount failed");
    }

    s_deviceId = getDeviceId();
    Serial.printf("\n[Main] WhisperBridge  id=%s\n", s_deviceId.c_str());

    Ble.setup();

    if (connectWifi()) {
        startStation();
    } else {
        startAP();
    }
}

void loop() {
    if (s_apMode) {
        s_dns.processNextRequest();
        pollScan();
        return;
    }

    ArduinoOTA.handle();
    Mqtt.loop();

    // Once the BLE task finishes, publish the OFF state
    static bool s_wasRunning = false;
    const bool  running      = Ble.isRunning();
    if (s_wasRunning && !running) {
        Mqtt.publishState(false);
    }
    s_wasRunning = running;
}
