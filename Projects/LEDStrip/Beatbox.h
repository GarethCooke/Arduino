#pragma once

#include <Arduino.h>
#include <set>
#include "SettingsListener.h"
#include "SoundEvent.h"


class Beatbox : public SettingsListener
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
	int	m_filterValue	= 94;	// MSGEQ7 OUT pin produces values around 50-80 when there is no input, so use this value to filter out a lot of the chaff.
	int	m_minbeatband	= 0;
	int	m_maxbeatband	= 0;
	static const constexpr uint32_t	m_ts = 18;
	static const constexpr uint32_t	m_to = 36;
	SoundEvent m_event;

	uint8_t m_reset_pin;
	uint8_t m_strobe_pin;
	uint8_t m_beatin_pin;
	std::set<SoundEvent::Listener*> m_listeners;

	Beatbox(uint8_t reset_pin, uint8_t strobe_pin, uint8_t beatin_pin);
	static void	handle(void* pvParameters);
	void		handleHardware();
	int			strobeHardware();
	void		handleSoftware();
};

