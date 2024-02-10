#include "../Fonts/MyFreeSans5pt7b.h"
#include "BeatDisplay.h"
#include <stdlib.h>

#define SCREEN_WIDTH 128	// OLED display width, in pixels
#define SCREEN_HEIGHT 32	// OLED display height, in pixels
#define OLED_RESET -1		// Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
class BeatVisualisationRoll : public BeatDisplay::BeatVisualisation
{
public:
	virtual void onBeat(unsigned long (&beats)[SoundEvent::getBands()]);

private:
	unsigned int m_beatNo = -1;
};

class BeatVisualisationStrobe : public BeatDisplay::BeatVisualisation
{
public:
	virtual void onBeat(unsigned long (&beats)[SoundEvent::getBands()]);
};

BeatDisplay::BeatDisplay(NetworkHost &host)
	: m_host(host), m_pBeatVisualisation(newVisualisation()), m_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET)
{
	if (!m_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
		Serial.println(F("SSD1306 allocation failed"));
}

BeatDisplay::~BeatDisplay()
{
	delete m_pBeatVisualisation;
}

void BeatDisplay::notify(const SoundEvent &evt)
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
	unsigned int bandWidth = getBandWidth(SoundEvent::getBands());

	if (now - m_lastRefresh > m_displayRefreshPeriod)
	{
		m_display.clearDisplay();
		m_display.setRotation(1);

		m_display.setTextColor(WHITE);

		(this->*RefreshMain)(evt, bandWidth);
		m_lastRefresh = now;
	}

	if (evt.beatDetected())
		m_pBeatVisualisation->onBeat(m_beats);

	int n = 0;
	for (auto &val : m_beats)
	{
		long beatPeriod = now - min(now, val);
		long beatDecay = 100 * beatPeriod / m_pBeatVisualisation->pulseLength();
		long decayAdjust = (m_topBeatSize * beatDecay / 100);
		if (beatPeriod <= m_pBeatVisualisation->pulseLength())
			m_display.fillRect(m_margin + decayAdjust / 2, getBandPos(n, bandWidth), m_topBeatSize - decayAdjust, bandWidth, WHITE);
		n++;
	}

	m_display.display();
}

unsigned int BeatDisplay::getBandWidth(unsigned int bands) const
{
	return (m_display.height() - 2 * m_margin - m_bandMargin * (bands - 1)) / bands;
}

int BeatDisplay::getBandPos(unsigned int band, unsigned int bandWidth) const
{
	return m_display.height() - bandWidth - band * (bandWidth + m_bandMargin);
}

void BeatDisplay::cycleDisplay()
{
	RefreshMain = (DisplayType::display_info == ++m_displayType) ? &BeatDisplay::displayInfo : &BeatDisplay::displayEqualiser;

	BeatDisplay::BeatVisualisation *pTemp = m_pBeatVisualisation;
	m_pBeatVisualisation = newVisualisation();
	delete pTemp;
}

BeatDisplay::DisplayType &operator++(BeatDisplay::DisplayType &val)
{
	val = static_cast<BeatDisplay::DisplayType>(static_cast<int>(val) + 1);

	if (val == BeatDisplay::DisplayType::display_null)
		val = static_cast<BeatDisplay::DisplayType>(0);
	return val;
}

BeatDisplay::BeatVisualisation *BeatDisplay::newVisualisation()
{
	if (DisplayType::display_beatroll != m_displayType)
		return new BeatVisualisationStrobe();
	else
		return new BeatVisualisationRoll();
}

void BeatDisplay::displayEqualiser(const SoundEvent &evt, unsigned int bandWidth)
{
	unsigned int maxBandSize = m_display.width() - 3 * m_margin - m_topBeatSize;
	unsigned int currentBand = 0;

	evt.iterate_bands([this, currentBand, bandWidth, maxBandSize](const String &frequency, unsigned int value) mutable -> void
					  {
        				unsigned int bandSize = maxBandSize * (value / 255.0);
       					m_display.fillRect(m_display.width() - m_margin - bandSize, getBandPos(currentBand++, bandWidth), bandSize, bandWidth, WHITE); });
}

void BeatDisplay::displayInfo(const SoundEvent &evt, unsigned int bandWidth)
{
	String hostname = m_host.getHostName();
	String mac = m_host.getMACAddress();
	String ip = m_host.getIPAddress();

	int16_t x1, y1;
	uint16_t w, h;
	uint8_t origRotation = m_display.getRotation();
	m_display.setRotation(0);

	m_display.setFont(&MyFreeSans5pt7b);
	m_display.getTextBounds(ip, m_margin, 0, &x1, &y1, &w, &h);

	m_display.setCursor(m_margin, 2 * m_margin + m_topBeatSize + 1 + h);
	m_display.print("Host: ");
	m_display.print(hostname);

	m_display.setCursor(m_margin, 3 * m_margin + m_topBeatSize + 1 + 2 * h);
	m_display.print("MAC: ");
	m_display.print(mac);

	m_display.setCursor(m_margin, 4 * m_margin + m_topBeatSize + 1 + 3 * h);
	m_display.print("IP: ");
	m_display.print(ip);

	m_display.setRotation(origRotation);
}

void BeatVisualisationRoll::onBeat(unsigned long (&beats)[SoundEvent::getBands()])
{
	if (++m_beatNo >= SoundEvent::getBands())
		m_beatNo = 0;
	beats[m_beatNo] = millis();
}

void BeatVisualisationStrobe::onBeat(unsigned long (&beats)[SoundEvent::getBands()])
{
	unsigned long now = millis();
	int midPoint = (SoundEvent::getBands() + 1) / 2;
	double decayPerBeat = 3.0 / static_cast<double>(SoundEvent::getBands() + 1);
	int n = 0;
	for (auto &val : beats)
	{
		beats[n] = now - (m_beatPulseLen * abs(n + 1 - midPoint) * decayPerBeat);
		n++;
	}
}
