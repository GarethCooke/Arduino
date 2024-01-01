#include <Adafruit_NeoPixel.h>
#include <WS2812FX.h>
#include <Arduino.h>
#include <memory>
#include <ArduinoJson.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#elif defined(ESP8266)
//#include <ESP8266WiFi.h>
//#include <ESPAsyncTCP.h>
#endif
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

#include "IguanaOTA.h"
#include <TwoStateValue.h>
#include "LEDStripController.h"
#include "LEDStripHTTPServ.h"
#include "BeatDisplay.h"
#include "LEDRGB.h"
#include "LEDRGBAddressable.h"
#include "PanelDisplay.h"


#define RESET_BTN_PERIOD	3000
#define PRE_RESET_PERIOD	2000

std::auto_ptr<LEDStripController>	pHub;
std::auto_ptr<LEDStripHTTPServ>		pHTTPServer;
std::auto_ptr<BeatDisplay>			pBeatDisplay;
std::auto_ptr<DigitalPinValue>		pCmd_btn;
std::auto_ptr<LEDRGB>				pStrip1;
std::auto_ptr<LEDRGBAddressable>	pStrip2;
std::auto_ptr<PanelDisplay>			pPanel1;


void setup(void)
{
	Serial.begin(9600);
	Serial.println("Starting...");

#ifdef ESP32
	uint8_t r_pin = 26;
	uint8_t g_pin = 25;
	uint8_t b_pin = 27;
	uint8_t reset_pin = 14;
	uint8_t strobe_pin = 12;
	uint8_t beatin_pin = 32;
	uint8_t cmndbtn_pin = 33;
#elif defined(ESP8266)
	uint8_t r_pin = D3;
	uint8_t g_pin = D7;
	uint8_t b_pin = D8;
	uint8_t reset_pin = D6;
	uint8_t strobe_pin = D4;
	uint8_t beatin_pin = A0;
	uint8_t cmndbtn_pin = D5;
#endif

	pHub.reset(new LEDStripController());
	pHTTPServer.reset(new LEDStripHTTPServ(*pHub));
	pBeatDisplay.reset(new BeatDisplay(*pHub));
	pCmd_btn.reset(new DigitalPinValue(cmndbtn_pin));
	pStrip1.reset(new LEDRGB(r_pin, g_pin, b_pin));
	pStrip2.reset(new LEDRGBAddressable(13));
	pPanel1.reset(new PanelDisplay(23, 18, 5, 4, 1));

	Beatbox::create(reset_pin, strobe_pin, beatin_pin);
	Beatbox& beatbox = Beatbox::get();

	beatbox.addListener(pBeatDisplay.get());
	beatbox.addListener(pStrip1.get());
	beatbox.addListener(pStrip2.get());
	beatbox.addListener(pPanel1.get());
	beatbox.addListener(pHTTPServer.get());

	pHub->addListener(&beatbox);
	pHub->addListener(pStrip1.get());
	pHub->addListener(pStrip2.get());
	pHub->addListener(pPanel1.get());

	Serial.println("resetFromSettings...");
	pHub->resetFromSettings();
	Serial.println("resetFromSettings complete");

	Serial.println("IguanaOTA::Initialise...");
	IguanaOTA::Initialise(pHub->getHostName());
	Serial.println("IguanaOTA::Initialise complete");

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
			pHub->factory_reset(); // we've flashed enough, now reset

	}
	else if (pCmd_btn->isSet())
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
}
