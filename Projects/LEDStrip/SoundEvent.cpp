#include "SoundEvent.h"

//const unsigned int SoundEvent::m_fequencies[] = { 63,160,400,1000,2500,6250,16000 };

const String SoundEvent::m_fequencies[] = { "63Hz", "160Hz", "400Hz", "1000Hz", "2500Hz", "6250Hz", "16000Hz" };

SoundEvent::SoundEvent() : m_beatDetected(false)
{
	memset(&m_results, 0, sizeof(m_results));
}


SoundEvent::Initialiser::Initialiser() : m_current(0), m_pEvent(new SoundEvent())
{
}


SoundEvent::Initialiser::~Initialiser()
{
	delete m_pEvent;
}


SoundEvent::Initialiser& SoundEvent::Initialiser::operator=(int value)
{
	if (m_current < sizeof(m_pEvent->m_results))
	{
		m_pEvent->m_results[m_current++] = value;
	}
	else
		Serial.println("SoundEvent initialisation - attempt to initialise more values than expected.");

	return *this;
}
