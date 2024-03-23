#include <../../IguanaFont/include/IguanaFreeSans5pt7b.h>
#include <BeatDisplay.h>
#include <stdlib.h>


class BeatVisualisationRoll : public BeatDisplay::BeatVisualisation
{
public:
	virtual void onBeat(unsigned long(&beats)[MSGEQ7Out::getBands()]);

private:
	unsigned int m_beatNo = -1;
};

class BeatVisualisationStrobe : public BeatDisplay::BeatVisualisation
{
public:
	virtual void onBeat(unsigned long(&beats)[MSGEQ7Out::getBands()]);
};

BeatDisplay::BeatDisplay(NetworkHost& host)
	: m_host(host), m_pBeatVisualisation(newVisualisation())
{
}

BeatDisplay::~BeatDisplay()
{
	delete m_pBeatVisualisation;
}

void BeatDisplay::notify(const MSGEQ7Out& evt)
{
	// display layout as follows...
	// |---------------------------------------------------------|
	// |------------------------- margin ------------------------|
	// |            top beat size with beat indicator #			 |
	// |------------------------- margin ------------------------|
	// |	#	|		|		|		| 		 |	#	 |		 |
	// |	#	|		|		|		| 		 |		 |		 |
	// |	#			|		|		| 		 |	b	 |		 |
	// |	#	b		|		|		| 		 |	a	 |		 |
	// |	#	a		| spectrum display area  |	n	 |		 |
	// |	#	n		|	    (example)		 |	d	 |		 |
	// |m	#	d	#	|		|		| 	#	 |	 	 |	#	m|
	// |a	#		#	|		|	#	| 	#	 |	s	 |	#	a|
	// |r	#	m	#	|	#	|	#	| 	#	 |	p	 |	#	r|
	// |g	#	a	#	|	#	|	#	| 	#	 |	e	 |	#	g|
	// |i	#	r	#	|	#	|	#	| 	#	 |	c	 |	#	i|
	// |n	#	g	#	|	#	|	#	| 	#	 |	t	 |	#	n|
	// |	#	i	#	|	#	|	#	| 	#	 |	r	 |	#	 |
	// |	#	n	#	|	#	|	#	| 	#	 |	u	 |	#	 |
	// |	#		#	|	#	|	#	| 	#	 |	m	 |	#	 |
	// |	#	|	#	|	#	|	#	| 	#	 |		 |	#	 |
	// |	#	|	#	|	#	|	#	| 	#	 |	#	 |	#	 |
	// |------------------------- margin ------------------------|
	// |---------------------------------------------------------|
	// but bear in mind that we rotate the display below before
	// using it so that x and y are rotated

	unsigned long now = millis();
	unsigned int bandWidth = getBandWidth(MSGEQ7Out::getBands());

	if (now - m_lastRefresh > m_displayRefreshPeriod)
	{
		resetDisplay();

		display().setRotation(1);

		display().setTextColor(getTextColour());


		(this->*RefreshMain)(evt, bandWidth);
		m_lastRefresh = now;
	}

	if (evt.beatDetected())
		m_pBeatVisualisation->onBeat(m_beats);

	int n = 0;
	for (auto& val : m_beats)
	{
		long beatPeriod = now - min(now, val);
		long beatDecay = 100 * beatPeriod / m_pBeatVisualisation->pulseLength();
		long decayAdjust = (m_topBeatSize * beatDecay / 100);
		if (beatPeriod <= m_pBeatVisualisation->pulseLength())
			display().fillRect(m_margin + decayAdjust / 2, getBandPos(n, bandWidth), m_topBeatSize - decayAdjust, bandWidth, getBarColour());
		n++;
	}

	show();
}

unsigned int BeatDisplay::getBandWidth(unsigned int bands) const
{
	return (height() - 2 * m_margin - m_bandMargin * (bands - 1)) / bands;
}

int BeatDisplay::getBandPos(unsigned int band, unsigned int bandWidth) const
{
	return height() - bandWidth - band * (bandWidth + m_bandMargin);
}

void BeatDisplay::cycleDisplay()
{
	RefreshMain = (DisplayType::display_info == ++m_displayType) ? &BeatDisplay::displayInfo : &BeatDisplay::displayEqualiser;

	BeatDisplay::BeatVisualisation* pTemp = m_pBeatVisualisation;
	m_pBeatVisualisation = newVisualisation();
	delete pTemp;
}

BeatDisplay::DisplayType& operator++(BeatDisplay::DisplayType& val)
{
	val = static_cast<BeatDisplay::DisplayType>(static_cast<int>(val) + 1);

	if (val == BeatDisplay::DisplayType::display_null)
		val = static_cast<BeatDisplay::DisplayType>(0);
	return val;
}

BeatDisplay::BeatVisualisation* BeatDisplay::newVisualisation()
{
	if (DisplayType::display_beatroll != m_displayType)
		return new BeatVisualisationStrobe();
	else
		return new BeatVisualisationRoll();
}

void BeatDisplay::displayEqualiser(const MSGEQ7Out& evt, unsigned int bandWidth)
{
	unsigned int maxBandSize = display().width() - 3 * m_margin - m_topBeatSize;
	unsigned int currentBand = 0;

	evt.iterate_bands([this, currentBand, bandWidth, maxBandSize](const char* frequency, unsigned int value, bool beat) mutable -> void
		{
			unsigned int bandSize = maxBandSize * (value / 255.0);
			display().fillRect(display().width() - m_margin - bandSize, getBandPos(currentBand++, bandWidth), bandSize, bandWidth, getBarColour()); });
}

void BeatDisplay::displayInfo(const MSGEQ7Out& evt, unsigned int bandWidth)
{
	String hostname = m_host.getHostName();
	String mac = m_host.getMACAddress();
	String ip = m_host.getIPAddress();

	int16_t x1, y1;
	uint16_t w, h;
	uint8_t origRotation = display().getRotation();
	display().setRotation(0);

	display().setFont(&IguanaFreeSans5pt7b);
	display().getTextBounds(ip, m_margin, 0, &x1, &y1, &w, &h);

	display().setCursor(m_margin, 2 * m_margin + m_topBeatSize + 1 + h);
	display().print("Host: ");
	display().print(hostname);

	display().setCursor(m_margin, 3 * m_margin + m_topBeatSize + 1 + 2 * h);
	display().print("IP: ");
	display().print(ip);

	display().setCursor(m_margin, 4 * m_margin + m_topBeatSize + 1 + 3 * h);
	display().print("MAC: ");
	display().print(mac);


	display().setRotation(origRotation);
}

void BeatVisualisationRoll::onBeat(unsigned long(&beats)[MSGEQ7Out::getBands()])
{
	if (++m_beatNo >= MSGEQ7Out::getBands())
		m_beatNo = 0;
	beats[m_beatNo] = millis();
}

void BeatVisualisationStrobe::onBeat(unsigned long(&beats)[MSGEQ7Out::getBands()])
{
	unsigned long now = millis();
	int midPoint = (MSGEQ7Out::getBands() + 1) / 2;
	double decayPerBeat = 3.0 / static_cast<double>(MSGEQ7Out::getBands() + 1);
	int n = 0;
	for (auto& val : beats)
	{
		beats[n] = now - (m_beatPulseLen * abs(n + 1 - midPoint) * decayPerBeat);
		n++;
	}
}
