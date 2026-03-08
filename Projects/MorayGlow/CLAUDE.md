# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

MorayGlow is an IoT LED strip controller. It has two separate sub-projects:

- **MorrayServer/** — ESP32-S3 firmware (C++, PlatformIO, Arduino framework) targeting the Seeed XIAO ESP32-S3
- **MorrayClient/** — Web UI served from the device (vanilla HTML/CSS/JS) + a Node.js dev mock server

## Build & Flash Commands (Firmware)

```bash
# Build firmware
pio run -e xiao_esp32s3

# Upload firmware to device
pio run -e xiao_esp32s3 -t upload

# Upload web UI files to LittleFS on device (run after firmware is on device)
pio run -e xiao_esp32s3 -t uploadfs

# Serial monitor
pio device monitor

# Run native C++ unit tests (no device needed)
pio test -e native
```

## Client Dev Server

```bash
cd MorrayClient/test-server
npm install   # first time only
npm start     # → http://localhost:3000  (mirrors firmware API exactly)
```

Client JS tests (Jest):
```bash
cd MorrayClient
npm test
```

Test-server JS tests (Jest):
```bash
cd MorrayClient/test-server
npm test
```

## Architecture

### Data & Control Flow

```
Home Assistant
    ↕ MQTT (morrayglow/state, morrayglow/command)
ESP32-S3 Firmware (MorrayServer)
    ↕ REST + WebSocket (/api/state, /api/power, /api/color, /ws)
Web Browser (MorrayClient)
```

State changes from any source (REST, MQTT, WebSocket) all call the same two functions in `main.cpp`: `applyLedState()` then `broadcastState()` then `mqttPublishState()`.

### Key Files

- **[include/state.h](MorrayServer/include/state.h)** — Shared state logic: `stateToJson`, `rgbToHex`, `parseMqttCommand`. Uses `#ifndef ARDUINO` guards so it compiles on native (for unit tests).
- **[include/mqtt.h](MorrayServer/include/mqtt.h)** — Header-only MQTT implementation with Home Assistant auto-discovery. All functions are `static` to avoid ODR violations.
- **[include/config.h](MorrayServer/include/config.h)** — WiFi credentials and MQTT broker — **edit before flashing**.
- **[js/morray.js](MorrayClient/js/morray.js)** — UMD utility module (works in browser and Node.js): `buildWsUrl`, `isValidHexColor`.
- **[test-server/logic.js](MorrayClient/test-server/logic.js)** — Server-side validation logic mirroring `state.h` in JavaScript.

### LED Hardware (main.cpp)

GPIO pin assignments on the XIAO ESP32-S3:

- `PIN_RED` (GPIO1), `PIN_GREEN` (GPIO2), `PIN_BLUE` (GPIO3) — PWM output via LEDC channels
- `PIN_STATUS` (GPIO4) — Status LED indicator
- `PIN_BUTTON` (GPIO5) — Button input with 50ms debounce and hold detection

`applyLedState()` drives the RGB strip via LEDC hardware PWM. The current firmware also includes:

- **Auto colour cycling**: cycles through 8 preset colours every 3 seconds
- **Button hold-to-reset**: hold 5 seconds to factory reset (status LED blinks faster as threshold approaches)
- **Boot blink**: 2-blink sequence on startup

### MQTT Payload Format

MQTT commands use Home Assistant JSON schema, which **differs from the REST API**:

```json
// MQTT command (subscribe):  morrayglow/command
{ "state": "ON", "color": { "r": 255, "g": 0, "b": 0 } }

// MQTT state (publish):      morrayglow/state
{ "on": true, "color": "#ff0000" }
```

HA auto-discovery is published to `homeassistant/light/morrayglow_01/config` on every MQTT reconnect.

### LittleFS

The web UI files (`index.html`, `css/`, `js/`) are uploaded to LittleFS on the device via `pio run -t uploadfs`. The `data/` directory is the source for this. The client has no build step — edit files directly.

## Library Conventions

- **ArduinoJson v7**: Use `JsonDocument` with no capacity argument (v7 is dynamic).
- **PubSubClient**: Always call `setBufferSize(512)` — default is too small for HA discovery payloads.
- **ESPAsyncWebServer**: POST body arrives in the `onBody` lambda (3rd arg to `server.on`), not the request handler.
