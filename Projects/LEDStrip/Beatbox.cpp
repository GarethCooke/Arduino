//#include <arduinoFFT.h>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include "Beatbox.h"

using std::bind;
using std::logic_error;

std::auto_ptr<Beatbox> pBeatbox;

//arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

//const uint16_t samples = 256; //This value MUST ALWAYS be a power of 2
//const double samplingFrequency = 10000;
//const unsigned int sampling_period_us = round(1000000 * (1.0 / samplingFrequency));

//double vReal[samples];
//double vImag[samples];

//const unsigned int Beatbox::Event::m_fequencies[] = { 63,160,400,1000,2500,6250,16000 };

const String Beatbox::Event::m_fequencies[] = { "63Hz", "160Hz", "400Hz", "1000Hz", "2500Hz", "6250Hz", "16000Hz" };

Beatbox::Event::Event() : m_beatDetected(false)
{
	memset(&m_results, 0, sizeof(m_results));
}

Beatbox::EventInitialiser& Beatbox::EventInitialiser::operator=(int value)
{
	if (m_current < sizeof(m_event.m_results))
	{
		m_event.m_results[m_current++] = value;
	}
	else
		Serial.println("Beatbox::Event initialisation - attempt to initialise more values than expected.");

	return *this;
}


void Beatbox::create(uint8_t reset_pin, uint8_t strobe_pin, uint8_t beatin_pin)
{
	pBeatbox.reset(new Beatbox(reset_pin, strobe_pin, beatin_pin));
}


Beatbox& Beatbox::get()
{
	if (!pBeatbox.get())
		throw logic_error("Attempt to get() Beatbox before call to create(...).");
	return *pBeatbox;
}


Beatbox::Beatbox(uint8_t reset_pin, uint8_t strobe_pin, uint8_t beatin_pin)
	:   m_reset_pin(reset_pin),
		m_strobe_pin(strobe_pin),
		m_beatin_pin(beatin_pin)
{
	pinMode(m_reset_pin,    OUTPUT);
	pinMode(m_strobe_pin,   OUTPUT);

  // Create an initial state for our pins
	digitalWrite(m_reset_pin, LOW);
	digitalWrite(m_strobe_pin, LOW);
	delay(1);

	// Reset the MSGEQ7 as per the datasheet timing diagram
	digitalWrite(m_reset_pin, HIGH);
	delay(1);
	digitalWrite(m_reset_pin, LOW);
	digitalWrite(m_strobe_pin, HIGH);
	delay(1);
}

void Beatbox::start()
{
	static auto created = false;
	if (created)
		return;

	created = true;
	static StaticTask_t xTaskBuffer;
	static const auto STACK_SIZE = 4000;
	static TaskHandle_t xHandle = NULL;
	//static StackType_t xStack[STACK_SIZE];

	xTaskCreate(handle, "Beatbox handler", STACK_SIZE, NULL, 5, &xHandle);
}


void Beatbox::notify(const JsonDocument& settings)
{
	// Copy values from the JsonDocument
	String filter           = settings["filter"];
	String beatdebounce     = settings["beatdebounce"];
	String gainlow          = settings["gainlow"];
	String gainincrement    = settings["gainincrement"];
	String gaindecrement    = settings["gaindecrement"];
	String minbeatband      = settings["minbeatband"];
	String maxbeatband      = settings["maxbeatband"];

	Serial.printf("Filter %s\n",			filter.c_str());
	Serial.printf("Beat debounce %s\n",		beatdebounce.c_str());
	Serial.printf("Gain low %s\n",			gainlow.c_str());
	Serial.printf("Gain increment %s\n",	gainincrement.c_str());
	Serial.printf("Gain decrement %s\n",	gaindecrement.c_str());
	Serial.printf("Min beat band %s\n",		minbeatband.c_str());
	Serial.printf("Max beat band %s\n",		maxbeatband.c_str());

	int nFilter         = atoi(filter.c_str());
	int nBeatdebounce   = atoi(beatdebounce.c_str());
	int nGainlow		= atoi(gainlow.c_str());
	int nGainincrement  = atoi(gainincrement.c_str());
	int nGaindecrement  = atoi(gaindecrement.c_str());
	int nMinbeatband	= atoi(minbeatband.c_str());
	int nMaxbeatband	= atoi(maxbeatband.c_str());

	// note that PWMRANGE is 0-1023 so we multiply the rgb values by 4
	resetParams(nFilter, nBeatdebounce, nGainlow, nGainincrement, nGaindecrement, nMinbeatband, nMaxbeatband);
}


void Beatbox::handle(void* pvParameters)
{
	if (!pBeatbox.get())
		throw logic_error("Attempt to use Beatbox handle(...) before call to create(...).");

	while(true)
		pBeatbox->handleHardware();
}


void Beatbox::handleHardware()
{
	// Set reset pin low to enable strobe
	digitalWrite(m_reset_pin, HIGH);
	delayMicroseconds(100);
	digitalWrite(m_reset_pin, LOW);

	EventInitialiser events;

	// Get all 7 spectrum values from the MSGEQ7
	for (int nSpectrum = 1; nSpectrum < 8; nSpectrum++)
	{
		int evt = strobeHardware();
		Serial.printf("Beat %d\n", evt);
		events = evt;

		if ((nSpectrum <= m_maxbeatband) && (nSpectrum <= m_minbeatband))
		{
			static unsigned long lastBeat = millis();
			
			m_beatAGC = constrain(m_beatAGC, 0, 255); // ensures the AGC is between the filter setting and the maximum
			if (evt >= m_AGCLowLimit)
			{
				if (evt > m_beatAGC)
				{
					if (!m_beatFlag)
					{
						events.signalBeatDetected();
						lastBeat = millis();
					}
					m_beatFlag = true;
					m_beatAGC = m_beatAGC + m_AGCInc; // increase the AGC if it's been set. Changing the values of the AGCInc & AGCDec (attack and decay) has a quite marked effect on the performance of the beat detector
				}
				else
				{
					//decrease the ACG if it hasn't
					m_beatAGC = m_beatAGC - m_AGCDec;
				}
			}

			if (!m_beatTimer-- || (millis() - lastBeat) > 50)
			{
//                static unsigned long last = millis();
//                Serial.printf("time taken for %d cycles: %lu\n", m_beatDebounce, millis() - last);
//                last = millis();
				m_beatFlag = false; // reset the beat flag
				m_beatTimer = m_beatDebounce;
			}
		}
	}

	const Event& event_to_publish = events;
	for_each(m_listeners.begin(), m_listeners.end(), [event_to_publish](std::set<EventListener*>::const_reference nextListener) { nextListener->notify(event_to_publish); });
}

int Beatbox::strobeHardware()
{
	digitalWrite(m_strobe_pin, LOW);
	delayMicroseconds(100); // Allow output to settle

	int evt = analogRead(m_beatin_pin);

	// Constrain any value above 1023 or below filterValue
	evt = constrain(evt, m_filterValue, 1023);

	// Remap the value to a number between 0 and 255
	evt = map(evt, m_filterValue, 1023, 0, 255);

	digitalWrite(m_strobe_pin, HIGH);
	delayMicroseconds(100); // Delay necessary due to timing diagram  
	yield();

	return evt;
}

void Beatbox::handleSoftware()
{
	//for (uint16_t i = 0; i < samples; i++)
	//{
	//    vReal[i] = analogRead(A0);
	//    static unsigned long oldTime = 0;
	//    unsigned long newTime = micros() - oldTime;
	//    oldTime = newTime;
	//    vReal[i] = analogRead(A0); // A conversion takes about 1mS on an ESP8266
	//    vImag[i] = 0;
	//    while (micros() < (newTime + sampling_period_us)) { /* do nothing to wait */ }
	//}
	//FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	/* Weigh data */
	//FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
	//FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */
	//Serial.println("Computed magnitudes:");

	//EventInitialiser events;
	//for (uint16_t nextFreq = 0, nextSample = 0; nextFreq < 7; nextFreq++)
	//{
	//    int evt = 0;
	//    while (nextSample < (samples >> 1))
	//    {
	//        double freq = ((nextSample * 1.0 * samplingFrequency) / samples);
	//        double val = vReal[nextSample++];
	//        if ((nextFreq = 6) || (abs(freq - static_cast<double>(Event::m_fequencies[nextFreq]))) < abs(freq - static_cast<double>(Event::m_fequencies[nextFreq + 1])))
	//            evt = max(evt, static_cast<int>(val));
	//    }
	//    events = evt;
	//}

	//Serial.println();
}


void Beatbox::addListener(EventListener* pListener)
{
	m_listeners.insert(pListener);
}


void Beatbox::removeListener(EventListener* pListener)
{
	m_listeners.erase(pListener);
}


void Beatbox::resetParams(int filterValue, int beatDebounce, int agcLowLimit, int agcInc, int agcDec, int nMinbeatband, int nMaxbeatband)
{
	m_filterValue   = filterValue;
	m_beatDebounce  = beatDebounce;
	m_AGCLowLimit   = agcLowLimit;
	m_AGCInc        = agcInc;
	m_AGCDec        = agcDec;
	m_minbeatband	= nMinbeatband;
	m_maxbeatband	= nMaxbeatband;
}
