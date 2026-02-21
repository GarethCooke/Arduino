#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "config.h"
#include "state.h"
#include "mqtt.h"

// ---- State ----
bool   ledOn    = false;
String ledColor = "#ffffff";

// ---- Server & WebSocket ----
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// ---- Helpers ----
void broadcastState() {
    ws.textAll(stateToJson(ledOn, ledColor));
}

void applyLedState() {
    // TODO: drive actual LED strip here
    Serial.printf("LED: %s  colour: %s\n", ledOn ? "ON" : "OFF", ledColor.c_str());
}

// ---- WebSocket event handler ----
void onWsEvent(AsyncWebSocket* srv, AsyncWebSocketClient* client,
               AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        client->text(stateToJson(ledOn, ledColor));
    }
}

// ---- Setup ----
void setup() {
    Serial.begin(115200);

    // LittleFS
    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed");
        return;
    }

    // WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected — IP: %s\n", WiFi.localIP().toString().c_str());

    // WebSocket
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    // GET /api/state
    server.on("/api/state", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send(200, "application/json", stateToJson(ledOn, ledColor));
    });

    // POST /api/power
    server.on("/api/power", HTTP_POST,
        [](AsyncWebServerRequest* req) {},
        nullptr,
        [](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t) {
            JsonDocument doc;
            if (deserializeJson(doc, data, len) || !doc["on"].is<bool>()) {
                req->send(400, "application/json", "{\"error\":\"missing 'on' boolean\"}");
                return;
            }
            ledOn = doc["on"].as<bool>();
            applyLedState();
            broadcastState();
            mqttPublishState();
            req->send(200, "application/json", stateToJson(ledOn, ledColor));
        }
    );

    // POST /api/color
    server.on("/api/color", HTTP_POST,
        [](AsyncWebServerRequest* req) {},
        nullptr,
        [](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t) {
            JsonDocument doc;
            if (deserializeJson(doc, data, len) || !doc["color"].is<const char*>()) {
                req->send(400, "application/json", "{\"error\":\"missing 'color'\"}");
                return;
            }
            ledColor = doc["color"].as<String>();
            applyLedState();
            broadcastState();
            mqttPublishState();
            req->send(200, "application/json", stateToJson(ledOn, ledColor));
        }
    );

    // Serve static files from LittleFS
    server.serveStatic("/", LittleFS, "/")
          .setDefaultFile("index.html")
          .setCacheControl("max-age=600");

    server.begin();
    Serial.println("HTTP server started");

    // MQTT
    mqttSetup();
}

// ---- Loop ----
void loop() {
    ws.cleanupClients();
    mqttLoop();
}
