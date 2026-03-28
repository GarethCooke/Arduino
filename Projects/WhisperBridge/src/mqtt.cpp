#include "mqtt.h"
#include "config.h"
#include "ble.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

MqttManager Mqtt;

// WiFiClient and PubSubClient kept file-scope to avoid pulling their headers into mqtt.h
static WiFiClient   s_wifiClient;
static PubSubClient s_mqtt(s_wifiClient);

// ── Private methods ───────────────────────────────────────────────────────────

void MqttManager::onMessage(const char* topic, uint8_t* payload, unsigned int length) {
    char msg[64] = {};
    memcpy(msg, payload, min((size_t)length, sizeof(msg) - 1));
    Serial.printf("[MQTT] %s → %s\n", topic, msg);

    if (strcmp(topic, MQTT_TOPIC_COMMAND) == 0) {
        if (strcmp(msg, MQTT_PAYLOAD_ON) == 0 || strcmp(msg, "1") == 0) {
            Mqtt.publishState(true);  // optimistic ON
            Ble.trigger();
            // loop() will publish OFF once Ble.isRunning() drops to false
        }
        // Ignore "OFF" — boost is a one-shot action, not a toggle
    }
}

void MqttManager::publishDiscovery() {
    JsonDocument doc;
    doc["name"]          = "Boost";
    doc["unique_id"]     = String("whisperbridge_") + _deviceId + "_boost";
    doc["command_topic"] = MQTT_TOPIC_COMMAND;
    doc["state_topic"]   = MQTT_TOPIC_STATE;
    doc["payload_on"]    = MQTT_PAYLOAD_ON;
    doc["payload_off"]   = MQTT_PAYLOAD_OFF;
    doc["optimistic"]    = false;
    doc["retain"]        = false;
    doc["icon"]          = "mdi:fan";

    JsonObject dev        = doc["device"].to<JsonObject>();
    dev["name"]           = "WhisperBridge";
    dev["model"]          = "WhisperBridge v1";
    dev["manufacturer"]   = "Custom";
    dev["identifiers"][0] = String("whisperbridge_") + _deviceId;

    char buf[512];
    serializeJson(doc, buf, sizeof(buf));
    s_mqtt.publish(MQTT_TOPIC_DISCOVERY, buf, /*retain=*/true);
    Serial.println("[MQTT] HA discovery published");
}

void MqttManager::connect() {
    String clientId = String("whisperbridge-") + _deviceId;
    bool ok = (strlen(MQTT_USER) > 0)
        ? s_mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)
        : s_mqtt.connect(clientId.c_str());

    if (ok) {
        Serial.println("[MQTT] Connected");
        s_mqtt.subscribe(MQTT_TOPIC_COMMAND);
        publishDiscovery();
        publishState(false);
    } else {
        Serial.printf("[MQTT] Connect failed, rc=%d\n", s_mqtt.state());
    }
}

// ── Public API ────────────────────────────────────────────────────────────────

void MqttManager::setup(const char* deviceId) {
    _deviceId = deviceId;
    s_mqtt.setServer(MQTT_HOST, MQTT_PORT);
    s_mqtt.setCallback(onMessage);
    s_mqtt.setBufferSize(512);
}

void MqttManager::loop() {
    if (!s_mqtt.connected()) {
        unsigned long now = millis();
        if (now - _lastReconnect >= 5000) {
            _lastReconnect = now;
            connect();
        }
    } else {
        s_mqtt.loop();
    }
}

void MqttManager::publishState(bool on) {
    s_mqtt.publish(MQTT_TOPIC_STATE, on ? MQTT_PAYLOAD_ON : MQTT_PAYLOAD_OFF, /*retain=*/true);
}
