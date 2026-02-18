#pragma once

// WiFi credentials
#define WIFI_SSID     "your-ssid"
#define WIFI_PASSWORD "your-password"

// MQTT broker
#define MQTT_HOST     "192.168.1.x"
#define MQTT_PORT     1883
#define MQTT_USER     ""
#define MQTT_PASSWORD ""

// Device identity
#define DEVICE_NAME "MorrayGlow"
#define DEVICE_ID   "morrayglow_01"

// MQTT topics
#define MQTT_TOPIC_STATE            "morrayglow/state"
#define MQTT_TOPIC_COMMAND          "morrayglow/command"
#define MQTT_TOPIC_DISCOVERY_PREFIX "homeassistant"
