#pragma once

#include <MSGEQ7Out.h>
#include <Adafruit_GFX.h>
#include "NetworkHost.h"

class BeatDisplay : public MSGEQ7Out::Listener
{
public:
	BeatDisplay(NetworkHost& host);
	virtual ~BeatDisplay();

	virtual void notify(const MSGEQ7Out& evt);

	void cycleDisplay();

	class BeatVisualisation
	{
	public:
		virtual void onBeat(unsigned long(&beats)[MSGEQ7Out::getBands()]) = 0;
		unsigned int pulseLength() const { return m_beatPulseLen; }

	protected:
		static const unsigned int m_beatPulseLen = 100;
	};

protected:
	virtual Adafruit_GFX& display() = 0;
	virtual void resetDisplay() = 0;
	virtual void show() = 0;
	virtual int16_t height() const = 0;
	virtual uint16_t getTextColour() const = 0;
	virtual uint16_t getBarColour() const = 0;

private:
	enum class DisplayType
	{
		display_beatstrobe = 0,
		display_beatroll,
		display_info,
		display_null
	};
	friend DisplayType& operator++(DisplayType& val);

	static const unsigned int m_margin = 1;
	static const unsigned int m_bandMargin = 1;
	static const unsigned long m_displayRefreshPeriod = 100;
	unsigned long m_lastRefresh = 0;
	unsigned int m_topBeatSize = 5;
	DisplayType m_displayType = DisplayType::display_beatstrobe;
	NetworkHost& m_host;
	BeatVisualisation* m_pBeatVisualisation;
	unsigned long m_beats[MSGEQ7Out::getBands()] = { 0 };

	unsigned int getBandWidth(unsigned int bands) const;
	int getBandPos(unsigned int band, unsigned int bandWidth) const;
	BeatVisualisation* newVisualisation();
	void displayEqualiser(const MSGEQ7Out& evt, unsigned int bandWidth);
	void displayInfo(const MSGEQ7Out& evt, unsigned int bandWidth);

	void (BeatDisplay::* RefreshMain)(const MSGEQ7Out& evt, unsigned int bandWidth) = &BeatDisplay::displayEqualiser;
};
