#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <WS2812FX.h>
#include <memory>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <EEPROM.h>
#include "FS.h"
#include <ESPAsyncWebSrv.h>
#include <Adafruit_I2CDevice.h>
#include "Update.h"
#include "ESPmDNS.h"
#include "ArduinoOTA.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MD_MAX72xx.h>
#include <MD_MAXPanel.h>
#include <BeatBroadcast.h>
#include <BeatWireSender.h>
#include <BeatUDPSender.h>
#include "IguanaOTA.h"
#include <ButtonStatus.h>
#include "LEDStripController.h"
#include "LEDStripHTTPServ.h"
#include "BeatDisplay.h"
#include "LEDRGB.h"
#include "LEDRGBAddressable.h"
#include "PanelDisplay.h"

#define RESET_BTN_PERIOD 3000
#define PRE_RESET_PERIOD 2000

std::unique_ptr<LEDStripController> pHub;
std::unique_ptr<BeatDisplay> pBeatDisplay;
std::unique_ptr<ButtonStatus> pCmd_btn;
std::unique_ptr<LEDRGB> pStrip1;
std::unique_ptr<LEDRGBAddressable> pStrip2;
std::unique_ptr<PanelDisplay> pPanel1;
std::unique_ptr<BeatBroadcast> pBroadcaster;

void setup(void)
{
	Serial.begin(115200);
	Serial.println("Starting...");

	constexpr const static uint8_t display_sda = 18;
	constexpr const static uint8_t display_scl = 9;
	constexpr const static uint8_t broadcast_sda = 21;
	constexpr const static uint8_t broadcast_scl = 20;
	constexpr const static uint8_t r_pin = 40;
	constexpr const static uint8_t g_pin = 41;
	constexpr const static uint8_t b_pin = 39;
	constexpr const static uint8_t beat_reset_pin = 5;
	constexpr const static uint8_t beat_strobe_pin = 2;
	constexpr const static uint8_t beat_in_pin = 1;
	constexpr const static uint8_t cmndbtn_pin = 4;
	constexpr const static uint8_t led_addr_data_pin = 42;
	constexpr const static uint8_t panel_pin_data = 45;
	constexpr const static uint8_t panel_pin_clk = 48;
	constexpr const static uint8_t panel_pin_cs = 47;
	constexpr const static uint8_t panel_devices_x = 4;
	constexpr const static uint8_t panel_devices_y = 1;

	Wire.begin(display_sda, display_scl);
	Wire1.begin(broadcast_sda, broadcast_scl);

	pHub.reset(new LEDStripController());
	LEDStripHTTPServ::create(*pHub);
	LEDStripHTTPServ& httpServer = LEDStripHTTPServ::get();
	pBeatDisplay.reset(new BeatDisplay(*pHub, Wire));
	pCmd_btn.reset(new ButtonStatus(shared_ptr_lite<TwoStateValue>(new DigitalPinValue(cmndbtn_pin))));
	pStrip1.reset(new LEDRGB(r_pin, g_pin, b_pin));
	pStrip2.reset(new LEDRGBAddressable(led_addr_data_pin));
	pPanel1.reset(new PanelDisplay(panel_pin_data, panel_pin_clk, panel_pin_cs, panel_devices_x, panel_devices_y));
	pBroadcaster.reset(new BeatBroadcast(unique_ptr<BeatSendImpl>(new BeatWireSender(Wire1))));
	// pBroadcaster.reset(new BeatBroadcast(unique_ptr<BeatSendImpl>(new BeatUDPSender())));

	Beatbox::create(beat_reset_pin, beat_strobe_pin, beat_in_pin);
	Beatbox& beatbox = Beatbox::get();

	beatbox.addListener(pBeatDisplay.get());
	beatbox.addListener(pStrip1.get());
	beatbox.addListener(pStrip2.get());
	beatbox.addListener(pPanel1.get());
	beatbox.addListener(pBroadcaster.get());
	beatbox.addListener(&httpServer);

	pHub->addListener(&beatbox);
	pHub->addListener(pStrip1.get());
	pHub->addListener(pStrip2.get());
	pHub->addListener(pPanel1.get());

	Serial.println("resetFromSettings...");
	pHub->resetFromSettings();

	Serial.println("IguanaOTA::Initialise...");
	IguanaOTA::Initialise(pHub->getHostName());

	Serial.println("starting beat box");
	beatbox.start();

	Serial.println("Ready");
}

void loop(void)
{
	static unsigned long btnDownStart = -1;
	static bool resetting = false;
	if (resetting)
	{
		// ok, we're resetting but first flash a bit to inform the user
		unsigned long now = millis();
		if (now - btnDownStart > RESET_BTN_PERIOD + PRE_RESET_PERIOD) // we're going to flash for one second
			pHub->factory_reset();									  // we've flashed enough, now reset
	}
	else if (pCmd_btn->isOn())
	{
		if (btnDownStart == -1)
			btnDownStart = millis();
		else if (millis() - btnDownStart > RESET_BTN_PERIOD) // if the button has been held down for three seconds
		{
			// reset to factory settings
			resetting = true;
		}
	}
	else if (btnDownStart != -1) // no longer holding button - ignore the previous press
	{
		btnDownStart = -1;
		pBeatDisplay->cycleDisplay();
	}

	IguanaOTA::handle();
	pStrip1->handle();
	pStrip2->handle();

	LEDStripHTTPServ::get().handle();
}
