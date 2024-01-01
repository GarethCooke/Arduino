#pragma once

#include <vector>
#include "Beatbox.h"

class LEDRGB : public  LEDStripController::SettingsListener, public Beatbox::EventListener
{
public:
	LEDRGB(uint8_t r_pin, uint8_t g_pin, uint8_t b_pin);
	virtual void notify(const JsonDocument& settings);
	virtual void notify(const Beatbox::Event& evt);

	void handle();

private:
	class RGB
	{
	public:
		RGB() {}
		RGB(unsigned long hexValue) : m_r(hexToR(hexValue)), m_g(hexToG(hexValue)), m_b(hexToB(hexValue)) {}

		unsigned int	r() const { return m_r; }
		unsigned int	g() const { return m_g; }
		unsigned int	b() const { return m_b; }

		void set_r(unsigned int r) { m_r = r; }
		void set_g(unsigned int g) { m_g = g; }
		void set_b(unsigned int b) { m_b = b; }

		static unsigned int hexToR(unsigned long hexValue) { return hexConvert(hexValue, 16);	}
		static unsigned int hexToG(unsigned long hexValue) { return hexConvert(hexValue, 8);	}
		static unsigned int hexToB(unsigned long hexValue) { return hexConvert(hexValue, 0);	}

	private:
		unsigned int m_r = 0;
		unsigned int m_g = 0;
		unsigned int m_b = 0;

		// note that PWMRANGE is 0-1023 so we multiply the rgb values by 4
		static unsigned int hexConvert(unsigned long hexValue, unsigned int shift) { return ((hexValue >> shift) & 0xFF) * 4; }
	};

	class CycleColor
	{
	public:
		enum DurationType
		{
			beat,
			time
		};

		CycleColor(unsigned long hexValue, unsigned int	duration, const char* durationType, unsigned int decay)
			:	m_rgb(hexValue),
				m_duration(duration),
				m_durationType(durationType && !strcmp(durationType, "time") ? DurationType::time : DurationType::beat),
				m_decay(decay)
		{
		}


		const RGB&		rgb()			const { return m_rgb;			}
		unsigned int	duration()		const { return m_duration;		}
		DurationType	durationType()	const { return m_durationType; }
		unsigned int	decay()			const { return m_decay;	}

	private:
		RGB				m_rgb;
		unsigned int	m_duration;
		DurationType	m_durationType;
		unsigned int	m_decay;

	};

	typedef std::vector<CycleColor> CycleColors;
	typedef std::pair<CycleColors::const_iterator, unsigned long> CurrentCycle;

#ifdef ESP32
	const int ledChannelR = 0;
	const int ledChannelG = 1;
	const int ledChannelB = 2;
#elif defined(ESP8266)
	uint8_t			m_r_pin;
	uint8_t			m_g_pin;
	uint8_t			m_b_pin;
#endif
	RGB				m_rgb;
	bool			m_power		= false;
	bool			m_beatbox	= false;
	unsigned int	m_beatdecay	= 100;
	unsigned long	m_lastPulse	= 0;
	CycleColors		m_cycleColours;
	CurrentCycle	m_currentCycle;

	void reset(unsigned int r, unsigned int g, unsigned int b, bool power, bool beatbox);
	const CurrentCycle makeColourCycle(CycleColors::const_iterator it) const;
	const RGB& getBeatColour() const;
	unsigned int getDecay() const;
	void setCurrentColourCycle(bool beatDetected);
};

