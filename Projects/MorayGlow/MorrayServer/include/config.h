#pragma once

// Access point name shown when no WiFi credentials are stored
#define AP_SSID "MorrayGlow-Setup"

// MQTT broker
#define MQTT_HOST     "192.168.1.x"
#define MQTT_PORT     1883
#define MQTT_USER     ""
#define MQTT_PASSWORD ""

// Device identity
#define DEVICE_NAME  "MorrayGlow"
#define DEVICE_ID    "morrayglow_01"
#define OTA_HOSTNAME DEVICE_NAME  // mDNS name used for OTA uploads

// MQTT topics
#define MQTT_TOPIC_STATE            "morrayglow/state"
#define MQTT_TOPIC_COMMAND          "morrayglow/command"
#define MQTT_TOPIC_DISCOVERY_PREFIX "homeassistant"
