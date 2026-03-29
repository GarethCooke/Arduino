#pragma once
#include <Arduino.h>

class MqttManager {
public:
    // Call once after WiFi connects. deviceId = last-3-MAC-bytes suffix.
    void setup(const char* deviceId);

    // Drive the MQTT client; call every loop iteration.
    void loop();

    // Publish ON or OFF to the state topic.
    void publishState(bool on);

private:
    void        connect();
    void        publishDiscovery();
    static void onMessage(const char* topic, uint8_t* payload, unsigned int length);

    String        _deviceId;
    unsigned long _lastReconnect = 0;
};

extern MqttManager Mqtt;
