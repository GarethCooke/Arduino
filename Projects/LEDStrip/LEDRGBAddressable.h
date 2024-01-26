#pragma once

#include "Beatbox.h"

class WS2812FX;

class LEDRGBAddressable : public  SettingsListener, public SoundEvent::Listener
{
public:
	LEDRGBAddressable(uint8_t d_pin);
	virtual ~LEDRGBAddressable();

	virtual void notify(const JsonDocument& settings);
	virtual void notify(const SoundEvent& evt);

	void handle();

private:
	WS2812FX* m_pWS2812fx = NULL;
};

