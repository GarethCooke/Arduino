#pragma once

// ── WiFi ─────────────────────────────────────────────────────────────────────
#define WIFI_HOSTNAME    "whisperbridge"
#define WIFI_AP_SSID     "WhisperBridge-Setup"
#define WIFI_AP_PASSWORD "whisperbridge"  // min 8 chars for WPA2

// ── MQTT ─────────────────────────────────────────────────────────────────────
// Edit before flashing
#define MQTT_HOST     "192.168.1.x"   // TODO: set broker IP
#define MQTT_PORT     1883
#define MQTT_USER     ""
#define MQTT_PASSWORD ""

// Topics
#define MQTT_TOPIC_COMMAND   "whisperbridge/boost"
#define MQTT_TOPIC_STATE     "whisperbridge/boost/state"
#define MQTT_TOPIC_DISCOVERY "homeassistant/switch/whisperbridge_boost/config"

// Payloads
#define MQTT_PAYLOAD_ON  "ON"
#define MQTT_PAYLOAD_OFF "OFF"

// ── BLE – Vent-Axia Svara ────────────────────────────────────────────────────
// TODO: replace with actual fan MAC address
#define FAN_MAC_ADDRESS "AA:BB:CC:DD:EE:FF"

// Service containing the PIN/auth characteristic
#define BLE_AUTH_SERVICE_UUID "e6834e4b-7b3a-48e6-91e4-f1d005f564d3"
#define BLE_AUTH_CHAR_UUID    "4cad343a-209a-40b7-b911-4d9b3df569b2"
// PIN value: e3146205 as raw bytes
#define BLE_PIN_BYTES         { 0xe3, 0x14, 0x62, 0x05 }

// Service containing the command characteristic
#define BLE_CMD_SERVICE_UUID  "c119e858-0531-4681-9674-5a11f0e53bb4"
#define BLE_CMD_CHAR_UUID     "118c949c-28c8-4139-b0b3-36657fd055a9"
// Boost command payload
#define BLE_BOOST_BYTES       { 0x01, 0x60, 0x09, 0x84, 0x03 }

// ── OTA ──────────────────────────────────────────────────────────────────────
#define OTA_PASSWORD "password"
