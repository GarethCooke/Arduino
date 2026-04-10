# Arduino

Arduino, ESP8266, and ESP32 projects.

## Standalone projects

Complete systems with their own repos, docs, and release history:

| Project | Description | Docs |
| ------- | ----------- | ---- |
| [MorayGlow](https://github.com/GarethCooke/MorayGlow) | WiFi RGB LED strip controller — XIAO ESP32-S3, custom PCB, MQTT/REST/WebSocket, Home Assistant | [garethcooke.github.io/MorayGlow](https://garethcooke.github.io/MorayGlow/) |
| [WhisperBridge](https://github.com/GarethCooke/WhisperBridge) | WiFi/MQTT-to-BLE bridge for Vent-Axia Svara fan, Home Assistant integration, no cloud required | [garethcooke.github.io/WhisperBridge](https://garethcooke.github.io/WhisperBridge/) |

## Experiments and prototypes

Smaller, exploratory projects that live here:

- **BeatReceiveTest** / **BeatReceiveWifi** — beat detection and WiFi sync experiments
- **EqualiserTest** — audio equaliser prototype
- **LEDStrip** — early LED strip work (preceded MorayGlow)
- **VentAxia** — Vent-Axia fan analysis (preceded WhisperBridge)

## Common libraries

Shared code used across projects lives in [`Common/`](Common/):

- **IguanaLib** — core utilities
- **IguanaFont** — font rendering helpers
- **IguanaBeat** — beat detection library
