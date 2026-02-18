#pragma once

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "state.h"

// Defined in main.cpp
extern bool   ledOn;
extern String ledColor;
extern void   applyLedState();
extern void   broadcastState();

static WiFiClient    _wifiClient;
static PubSubClient  mqttClient(_wifiClient);

static void publishDiscovery() {
    JsonDocument doc;
    doc["name"]            = DEVICE_NAME;
    doc["unique_id"]       = DEVICE_ID;
    doc["state_topic"]     = MQTT_TOPIC_STATE;
    doc["command_topic"]   = MQTT_TOPIC_COMMAND;
    doc["schema"]          = "json";

    JsonArray modes = doc["supported_color_modes"].to<JsonArray>();
    modes.add("rgb");

    JsonObject device = doc["device"].to<JsonObject>();
    device["identifiers"][0] = DEVICE_ID;
    device["name"]           = DEVICE_NAME;
    device["model"]          = "Wemos D1 Mini";
    device["manufacturer"]   = "MorrayGlow";

    String payload;
    serializeJson(doc, payload);

    String topic = String(MQTT_TOPIC_DISCOVERY_PREFIX) + "/light/" + DEVICE_ID + "/config";
    mqttClient.publish(topic.c_str(), payload.c_str(), true);
}

static void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

    bool   newOn    = ledOn;
    String newColor = ledColor;
    if (!parseMqttCommand(msg, newOn, newColor)) return;

    ledOn    = newOn;
    ledColor = newColor;

    applyLedState();
    broadcastState();
    mqttClient.publish(MQTT_TOPIC_STATE, stateToJson(ledOn, ledColor).c_str(), true);
}

static void mqttReconnect() {
    if (mqttClient.connected()) return;
    String clientId = String(DEVICE_ID) + "-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD,
                           MQTT_TOPIC_STATE, 0, true, "{\"on\":false}")) {
        mqttClient.subscribe(MQTT_TOPIC_COMMAND);
        publishDiscovery();
        mqttClient.publish(MQTT_TOPIC_STATE, stateToJson(ledOn, ledColor).c_str(), true);
    }
}

void mqttSetup() {
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setBufferSize(512);
}

void mqttLoop() {
    if (!mqttClient.connected()) {
        static unsigned long lastAttempt = 0;
        if (millis() - lastAttempt > 5000) {
            lastAttempt = millis();
            mqttReconnect();
        }
    }
    mqttClient.loop();
}

void mqttPublishState() {
    if (mqttClient.connected()) {
        mqttClient.publish(MQTT_TOPIC_STATE, stateToJson(ledOn, ledColor).c_str(), true);
    }
}
