#pragma once

#include "Beatbox.h"

class WS2812FX;

class LEDRGBAddressable : public  LEDStripController::SettingsListener, public Beatbox::EventListener
{
public:
	LEDRGBAddressable(uint8_t d_pin);
	virtual ~LEDRGBAddressable();

	virtual void notify(const JsonDocument& settings);
	virtual void notify(const Beatbox::Event& evt);

	void handle();

private:
	WS2812FX* m_pWS2812fx = NULL;
};

