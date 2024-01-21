#pragma once

#include <Arduino.h>
#include <set>
#include "LEDStripController.h"
#include "SoundEvent.h"


class Beatbox : public LEDStripController::SettingsListener
{
public:
	static void create(uint8_t reset_pin, uint8_t strobe_pin, uint8_t beatin_pin);
	static Beatbox& get();

	void notify(const JsonDocument& settings);
	void start();

	void addListener(SoundEvent::Listener* pListener);
	void removeListener(SoundEvent::Listener* pListener);

	void resetParams(int filterValue, int beatDebounce, int agcLowLimit, int agcInc, int agcDec, int nMinbeatband, int nMaxbeatband);

private:
	int m_filterValue	= 94;	// MSGEQ7 OUT pin produces values around 50-80 when there is no input, so use this value to filter out a lot of the chaff.
	int m_beatDebounce	= 30;	// debounce delay for the beat detector
	int m_AGCLowLimit	= 80;	// the lower limit of the beat detector AGC. Has a dramatic effect on beat detection.
	int m_AGCInc		= 100;	// the rate at which the AGC is incremented (attack)
	int m_AGCDec		= 10;	// the rate at which the AGC is decremented (decay)
	int m_minbeatband	= 0;
	int m_maxbeatband	= 0;

	uint8_t m_reset_pin;
	uint8_t m_strobe_pin;
	uint8_t m_beatin_pin;
	std::set<SoundEvent::Listener*> m_listeners;

	int				m_beatAGC = 126;					// beat detect AGC is halfway
	int				m_beatTimer = m_beatDebounce / 2;	// counter for the beat debounce
	bool			m_beatFlag = false;

	Beatbox(uint8_t reset_pin, uint8_t strobe_pin, uint8_t beatin_pin);
	static void	handle(void* pvParameters);
	void		handleHardware();
	int			strobeHardware();
	void		handleSoftware();
};

