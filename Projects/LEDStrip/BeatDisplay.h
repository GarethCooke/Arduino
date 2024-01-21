#pragma once

#include <vector>
#include <Adafruit_SSD1306.h>
#include "SoundEvent.h"
#include "NetworkHost.h"

class BeatDisplay : public SoundEvent::Listener
{
public:
	BeatDisplay(NetworkHost& host);
	virtual ~BeatDisplay();
	
	virtual void notify(const SoundEvent& evt);

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
		display_beatstrobe = 0,
		display_beatroll,
		display_info,
		display_null
	};
	friend DisplayType& operator++(DisplayType& val);

	static const unsigned int	m_margin				= 1;
	static const unsigned int	m_bandMargin			= 1;
	static const unsigned long	m_displayRefreshPeriod	= 100;
	unsigned long				m_lastRefresh			= 0;
	unsigned int				m_topBeatSize			= 5;
	DisplayType					m_displayType			= DisplayType::display_beatstrobe;
	NetworkHost&				m_host;
	BeatVisualisation*			m_pBeatVisualisation;
	std::vector<unsigned long>	m_beats;
	Adafruit_SSD1306			m_display;

	unsigned int getBandWidth(unsigned int bands) const;
	int getBandPos(unsigned int band, unsigned int bandWidth) const;
	BeatVisualisation* newVisualisation();
	void displayEqualiser(const SoundEvent& evt, unsigned int bandWidth);
	void displayInfo(const SoundEvent& evt, unsigned int bandWidth);

	void (BeatDisplay::*RefreshMain)(const SoundEvent& evt, unsigned int bandWidth) = &BeatDisplay::displayEqualiser;
};
