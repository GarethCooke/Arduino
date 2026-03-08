#pragma once

#include <ArduinoJson.h>
#include <AsyncWebSocket.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <WiFiConfig.h>

#include "config.h"
#include "state.h"

// Defined in main.cpp
extern bool   ledOn;
extern String ledColor;
extern bool   apMode;
extern void   applyLedState();
extern void   mqttPublishState();

static AsyncWebServer _server(80);
static AsyncWebSocket _ws("/ws");

// Called from mqtt.h (declared extern there)
void broadcastState() {
    _ws.textAll(stateToJson(ledOn, ledColor));
}

// ── Body handlers ────────────────────────────────────────────────────────────

static void onBody_power(AsyncWebServerRequest* req, uint8_t* data, size_t len,
                         size_t index, size_t total) {
    if (index + len < total) return;
    JsonDocument doc;
    if (deserializeJson(doc, (char*)data, len) != DeserializationError::Ok ||
        !doc["on"].is<bool>()) {
        req->send(400, "application/json", "{\"error\":\"on required\"}");
        return;
    }
    ledOn = doc["on"].as<bool>();
    applyLedState();
    broadcastState();
    mqttPublishState();
    req->send(200, "application/json", stateToJson(ledOn, ledColor));
}

static void onBody_color(AsyncWebServerRequest* req, uint8_t* data, size_t len,
                         size_t index, size_t total) {
    if (index + len < total) return;
    JsonDocument doc;
    if (deserializeJson(doc, (char*)data, len) != DeserializationError::Ok) {
        req->send(400, "application/json", "{\"error\":\"invalid JSON\"}");
        return;
    }
    const char* color = doc["color"] | "";
    if (strlen(color) != 7 || color[0] != '#') {
        req->send(400, "application/json", "{\"error\":\"invalid color\"}");
        return;
    }
    ledColor = color;
    applyLedState();
    broadcastState();
    mqttPublishState();
    req->send(200, "application/json", stateToJson(ledOn, ledColor));
}

static void onBody_networkset(AsyncWebServerRequest* req, uint8_t* data, size_t len,
                              size_t index, size_t total) {
    if (index + len < total) return;
    JsonDocument doc;
    if (deserializeJson(doc, (char*)data, len) != DeserializationError::Ok) {
        req->send(400, "application/json", "{\"error\":\"invalid JSON\"}");
        return;
    }
    const char* ssid = doc["ssid"] | "";
    const char* pwd  = doc["password"] | "";
    if (strlen(ssid) == 0) {
        req->send(400, "application/json", "{\"error\":\"ssid required\"}");
        return;
    }
    WiFiConfig cfg;
    cfg.setSSID(ssid);
    cfg.setPassword(pwd);
    EEPROM.put(0, cfg);
    EEPROM.commit();
    req->send(200, "application/json", "{\"ok\":true}");
    delay(500);
    ESP.restart();
}

// ── Setup / loop ─────────────────────────────────────────────────────────────

void webserverSetup() {
    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed");
    }

    _ws.onEvent([](AsyncWebSocket*, AsyncWebSocketClient* client, AwsEventType type,
                   void*, uint8_t*, size_t) {
        if (type == WS_EVT_CONNECT) client->text(stateToJson(ledOn, ledColor));
    });
    _server.addHandler(&_ws);

    // Network setup endpoints — available in both modes so WiFi can be
    // reconfigured from the main UI without requiring a factory reset.
    _server.on("/api/networkdata", HTTP_GET, [](AsyncWebServerRequest* req) {
        int        n   = WiFi.scanNetworks();
        JsonDocument doc;
        JsonArray    arr = doc["networks"].to<JsonArray>();
        for (int i = 0; i < n; i++) {
            JsonObject net = arr.add<JsonObject>();
            net["ssid"]    = WiFi.SSID(i).c_str();
            net["rssi"]    = WiFi.RSSI(i);
            net["secure"]  = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
        }
        String out;
        serializeJson(doc, out);
        req->send(200, "application/json", out);
    });

    _server.on("/api/networkset", HTTP_POST,
               [](AsyncWebServerRequest* req) {}, nullptr, onBody_networkset);

    if (apMode) {
        // In AP mode redirect everything to the setup page
        _server.serveStatic("/", LittleFS, "/").setDefaultFile("setup.html");
        _server.onNotFound([](AsyncWebServerRequest* req) {
            if (req->url() == "/setup.html")
                req->send(503, "text/plain", "LittleFS not mounted — run uploadfs");
            else
                req->redirect("/setup.html");
        });
    } else {
        _server.on("/api/state", HTTP_GET, [](AsyncWebServerRequest* req) {
            req->send(200, "application/json", stateToJson(ledOn, ledColor));
        });

        _server.on("/api/power", HTTP_POST,
                   [](AsyncWebServerRequest* req) {}, nullptr, onBody_power);

        _server.on("/api/color", HTTP_POST,
                   [](AsyncWebServerRequest* req) {}, nullptr, onBody_color);

        _server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    }

    _server.begin();
    Serial.println("HTTP server started");
}

void webserverLoop() {
    _ws.cleanupClients();
}
