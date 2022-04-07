#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Beatbox.h"

class BeatDisplay : public Beatbox::EventListener
{
public:
	BeatDisplay();
	virtual ~BeatDisplay();
	
	virtual void notify(const Beatbox::Event& evt);

	void cycleDisplay();

	class BeatVisualisation
	{
	public:
		virtual void	onBeat(std::vector<unsigned long>& beats) = 0;
		unsigned int	pulseLength() const { return m_beatPulseLen; }

	protected:
		static const unsigned int m_beatPulseLen = 1000;
	};

private:
	enum class DisplayType
	{
		display_beatroll = 0,
		display_beatstrobe,
		display_null
	};
	friend DisplayType operator++(DisplayType& val, int);

	static const unsigned int	m_margin				= 1;
	static const unsigned int	m_bandMargin			= 1;
	static const unsigned long	m_displayRefreshPeriod	= 100;
	unsigned long				m_lastRefresh			= 0;
	unsigned int				m_topBeatSize			= 5;
	DisplayType					m_displayType			= DisplayType::display_beatroll;
	BeatVisualisation*			m_pBeatVisualisation;
	std::vector<unsigned long>	m_beats;
	Adafruit_SSD1306			m_display;

	unsigned int getBandWidth(unsigned int bands) const;
	int getBandPos(unsigned int band, unsigned int bandWidth) const;
	BeatVisualisation* newVisualisation();
};
