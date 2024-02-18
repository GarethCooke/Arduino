#pragma once

#include "Beatbox.h"

class WS2812FX;

class LEDRGBAddressable : public SettingsListener, public MSGEQ7Out::Listener
{
public:
	LEDRGBAddressable(uint8_t d_pin);
	virtual ~LEDRGBAddressable();

	virtual void notify(const JsonDocument& settings);
	virtual void notify(const MSGEQ7Out& evt);

	void handle();

private:
	WS2812FX* m_pWS2812fx = NULL;
};
