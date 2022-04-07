#pragma once

#include <Arduino.h>
#include <set>
#include "LEDStripController.h"


class Beatbox : public LEDStripController::SettingsListener
{
public:
	Beatbox(uint8_t reset_pin, uint8_t strobe_pin, uint8_t beatin_pin);
	void notify(const JsonDocument& settings);
	void handle();

	class EventInitialiser;
	class Event
	{
		friend class EventInitialiser;

	public:
		unsigned int	getCount()							const { return 7; }
		const String&	getFrequency(unsigned int nIndex)	const { return m_fequencies[assureIndex(nIndex)];	}
		int				getValue(unsigned int nIndex)		const { return m_results[assureIndex(nIndex)];		}

		class BandIterator
		{
		public:
			BandIterator(const Event& evt, unsigned int current = 0) : m_evt(evt), m_current(current) {}
			const BandIterator& operator++() { m_current++; return *this; }
			std::pair<const String*, int> operator*() { return std::make_pair(&m_evt.m_fequencies[m_current], m_evt.m_results[m_current]); }
			bool operator==(const BandIterator& right) const { return m_current == right.m_current && &m_evt == &right.m_evt; }
			bool operator!=(const BandIterator& right) const { return !(*this == right); }
		private:
			const Event&	m_evt;
			unsigned int	m_current;
		};

		BandIterator	begin()			const	{ return BandIterator(*this);				}
		BandIterator	end()			const	{ return BandIterator(*this, getCount());	}
		bool			beatDetected()	const	{ return m_beatDetected;					}

	private:
		Event();

		unsigned int	assureIndex(unsigned int nIndex) const	{ return min(nIndex, getCount() - 1);	}
		void			signalBeatDetected()					{ m_beatDetected = true;				}

		static const String	m_fequencies[7];
		int					m_results[7];
		bool				m_beatDetected;
	};

	class EventInitialiser
	{
	public:
		EventInitialiser() : m_current(0) {}
		EventInitialiser& operator=(int value);
		operator const Event&() const	{ return m_event; }
		void signalBeatDetected()		{ m_event.signalBeatDetected(); }

	private:
		Event			m_event;
		unsigned int	m_current;
	};

	class EventListener
	{
	public:
		virtual void notify(const Event& evt) = 0;
	};

	void addListener(EventListener* pListener);
	void removeListener(EventListener* pListener);

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
	std::set<EventListener*> m_listeners;

	int				m_beatAGC = 126;					// beat detect AGC is halfway
	int				m_beatTimer = m_beatDebounce / 2;	// counter for the beat debounce
	bool			m_beatFlag = false;

	void	handleHardware();
	int		strobeHardware();
	void	handleSoftware();
};

