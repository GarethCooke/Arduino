// MorayGlow — XIAO ESP32-S3 RGB LED Strip Controller
// Hardware: Seeed XIAO ESP32-S3, IRLML6344TR MOSFETs, MP1584 buck, 12V 5050 RGB strip
// See docs/Hardware/ for schematic, netlist, and BOM.
//
// PIN MAPPING
//   PIN_RED    GPIO1  D0  LEDC CH0  PWM 1kHz 8-bit
//   PIN_GREEN  GPIO2  D1  LEDC CH1  PWM 1kHz 8-bit
//   PIN_BLUE   GPIO3  D2  LEDC CH2  PWM 1kHz 8-bit
//   PIN_STATUS GPIO4  D3  Output    Status LED (via 330Ω to GND)
//   PIN_BUTTON GPIO5  D4  Input     INPUT_PULLUP · hold 5s = factory reset
//
// BOOT BEHAVIOUR
//   1. Reads WiFi credentials from EEPROM (via IguanaLib WiFiConfig).
//   2. If valid credentials found → connects to home network (station mode).
//   3. If no credentials or connection fails → starts "MorrayGlow-Setup" AP.
//      Connect to that AP and visit http://192.168.4.1 to configure WiFi.
//   4. Hold button 5 s → clears saved credentials and restarts into AP mode.

#include <Arduino.h>
#include <ButtonStatus.h>
#include <EEPROM.h>
#include <TwoStateValue.h>
#include <WiFi.h>
#include <WiFiConfig.h>

#include "IguanaOTA.h"
#include "config.h"
#include "state.h"
#include "webserver.h"  // defines broadcastState()
#include "mqtt.h"       // uses extern broadcastState()

#define PIN_RED    1
#define PIN_GREEN  2
#define PIN_BLUE   3
#define PIN_STATUS 4
#define PIN_BUTTON 5

#define LEDC_CH_RED     0
#define LEDC_CH_GREEN   1
#define LEDC_CH_BLUE    2
#define LEDC_FREQ_HZ    1000
#define LEDC_RESOLUTION 8

#define HOLD_DURATION_MS          5000
#define FLASH_COUNT               5
#define FLASH_ON_MS               150
#define FLASH_OFF_MS              150
#define COLOUR_CHANGE_INTERVAL_MS 3000

struct Colour {
    uint8_t r, g, b;
};

const Colour colourSequence[] = {
    {255, 0, 0},      // Red
    {0, 255, 0},      // Green
    {0, 0, 255},      // Blue
    {255, 255, 0},    // Yellow
    {0, 255, 255},    // Cyan
    {255, 0, 255},    // Magenta
    {255, 255, 255},  // White
    {255, 128, 0},    // Amber
};
const uint8_t NUM_COLOURS   = sizeof(colourSequence) / sizeof(colourSequence[0]);
uint8_t       currentColour = 0;

// ── LED state — referenced as extern by webserver.h and mqtt.h ───────────────
bool   ledOn    = false;
String ledColor = "#ffffff";
bool   apMode   = false;

ButtonStatus* pBtn         = nullptr;
uint32_t      lastColourChange = 0;

// ── Hardware helpers ─────────────────────────────────────────────────────────

void setColour(uint8_t r, uint8_t g, uint8_t b) {
    ledcWrite(LEDC_CH_RED, r);
    ledcWrite(LEDC_CH_GREEN, g);
    ledcWrite(LEDC_CH_BLUE, b);
}

// Called from webserver.h and mqtt.h (declared extern there)
void applyLedState() {
    if (!ledOn) {
        setColour(0, 0, 0);
        return;
    }
    long c = strtol(ledColor.c_str() + 1, nullptr, 16);
    setColour((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
}

// Blink status LED, clear saved WiFi credentials, restart into AP mode.
void flashAndReset() {
    setColour(0, 0, 0);
    for (int i = 0; i < FLASH_COUNT; i++) {
        digitalWrite(PIN_STATUS, HIGH);
        delay(FLASH_ON_MS);
        digitalWrite(PIN_STATUS, LOW);
        delay(FLASH_OFF_MS);
    }
    // Zero out the WiFiConfig block so isValid() returns false on next boot
    for (size_t i = 0; i < sizeof(WiFiConfig); i++) EEPROM.write(i, 0);
    EEPROM.commit();
    delay(200);
    ESP.restart();
}

// ── Network setup ─────────────────────────────────────────────────────────────

void setupNetwork() {
    EEPROM.begin(512);
    WiFiConfig cfg;
    EEPROM.get(0, cfg);

    if (cfg.isValid() && cfg.hasSSID()) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(cfg.getSSID(), cfg.getPassword());
        Serial.printf("Connecting to %s", cfg.getSSID());
        uint32_t start    = millis();
        bool     ledState = false;
        while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
            delay(250);
            ledState = !ledState;
            digitalWrite(PIN_STATUS, ledState ? HIGH : LOW);
            Serial.print(".");
        }
        if (WiFi.status() == WL_CONNECTED) {
            digitalWrite(PIN_STATUS, HIGH);
            Serial.printf("\nWiFi connected — IP: %s\n",
                          WiFi.localIP().toString().c_str());
            IguanaOTA::Initialise(OTA_HOSTNAME);
            mqttSetup();
            return;
        }
        Serial.println("\nWiFi connection failed — entering setup mode");
    } else {
        Serial.println("No WiFi credentials — entering setup mode");
    }

    // Fall back to AP mode
    apMode = true;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID);
    Serial.printf("AP started — connect to '%s' and visit http://%s\n",
                  AP_SSID, WiFi.softAPIP().toString().c_str());

    // Slow blink to indicate AP mode (distinct from boot blink)
    for (int i = 0; i < 4; i++) {
        digitalWrite(PIN_STATUS, HIGH);
        delay(500);
        digitalWrite(PIN_STATUS, LOW);
        delay(500);
    }
}

// ── Arduino lifecycle ────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    pinMode(PIN_STATUS, OUTPUT);

    setupNetwork();

    ledcSetup(LEDC_CH_RED, LEDC_FREQ_HZ, LEDC_RESOLUTION);
    ledcSetup(LEDC_CH_GREEN, LEDC_FREQ_HZ, LEDC_RESOLUTION);
    ledcSetup(LEDC_CH_BLUE, LEDC_FREQ_HZ, LEDC_RESOLUTION);
    ledcAttachPin(PIN_RED, LEDC_CH_RED);
    ledcAttachPin(PIN_GREEN, LEDC_CH_GREEN);
    ledcAttachPin(PIN_BLUE, LEDC_CH_BLUE);

    // DigitalPinValue sets INPUT in its ctor; override immediately to INPUT_PULLUP.
    // highOn=false because the pin is active-low (INPUT_PULLUP, pressed = LOW).
    pBtn = new ButtonStatus(
        make_shared_ptr_lite<TwoStateValue>(new DigitalPinValue(PIN_BUTTON, false)));
    pinMode(PIN_BUTTON, INPUT_PULLUP);

    webserverSetup();

    // Boot flash: 2 blinks on status LED
    for (int i = 0; i < 2; i++) {
        digitalWrite(PIN_STATUS, HIGH);
        delay(200);
        digitalWrite(PIN_STATUS, LOW);
        delay(200);
    }

    applyLedState();
    digitalWrite(PIN_STATUS, HIGH);
    lastColourChange = millis();
}

void loop() {
    IguanaOTA::handle();
    webserverLoop();
    if (!apMode) mqttLoop();

    uint32_t now = millis();

    // ── Hold-to-reset: blink faster as hold approaches 5 s ──────────────────
    static uint32_t btnDownStart = 0;
    if (pBtn->isOn()) {
        if (btnDownStart == 0) btnDownStart = now;
        uint32_t held      = now - btnDownStart;
        uint32_t blinkRate = map(constrain(held, 0, HOLD_DURATION_MS),
                                 0, HOLD_DURATION_MS, 800, 80);
        digitalWrite(PIN_STATUS, ((now / blinkRate) % 2) ? HIGH : LOW);
        if (held >= HOLD_DURATION_MS) flashAndReset();
    } else {
        btnDownStart = 0;
        digitalWrite(PIN_STATUS, HIGH);
    }

    // ── Auto colour cycle every 3 s (paused while button held) ───────────────
    if (!pBtn->isOn() && (now - lastColourChange >= COLOUR_CHANGE_INTERVAL_MS)) {
        currentColour        = (currentColour + 1) % NUM_COLOURS;
        const Colour& c      = colourSequence[currentColour];
        ledOn                = true;
        ledColor             = rgbToHex(c.r, c.g, c.b);
        applyLedState();
        broadcastState();
        if (!apMode) mqttPublishState();
        lastColourChange = now;
    }
}
