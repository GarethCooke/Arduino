#include "LEDRGB.h"


LEDRGB::LEDRGB(uint8_t r_pin, uint8_t g_pin, uint8_t b_pin)
#ifdef ESP32
#elif defined(ESP8266)
	:	m_r_pin(r_pin),
		m_g_pin(g_pin),
		m_b_pin(b_pin)
#endif
{
	pinMode(r_pin, OUTPUT);
	pinMode(g_pin, OUTPUT);
	pinMode(b_pin, OUTPUT);

#ifdef ESP32
	const int freq = 5000;
	const int resolution = 10;

	ledcSetup(ledChannelR, freq, resolution);
	ledcSetup(ledChannelG, freq, resolution);
	ledcSetup(ledChannelB, freq, resolution);

	ledcAttachPin(r_pin, ledChannelR);
	ledcAttachPin(g_pin, ledChannelG);
	ledcAttachPin(b_pin, ledChannelB);
#endif
}


void LEDRGB::notify(const JsonDocument& settings)
{
	// Copy values from the Jsonsettingsument
	bool power			= settings["power"];
	String colour		= settings["colour"];
	String lightsetting = settings["lightsetting"];
	String beatdecay	= settings["beatdecay"];

	Serial.printf("Power %d\n", power);
	Serial.printf("Colour %s\n", colour.c_str());
	Serial.printf("Light setting %s\n", lightsetting.c_str());
	Serial.printf("Beat decay %s\n", beatdecay.c_str());

	unsigned long hexValue = strtol(&colour.c_str()[1], NULL, 16);
	m_beatdecay = atoi(beatdecay.c_str());

	JsonDocument copy_settings = settings;
	JsonArray cycle_colours = copy_settings["cyclecolours"];

	m_cycleColours.clear();

	for (JsonVariant v : cycle_colours)
	{
		int duration = v["duration"].as<int>();
		if (duration > 0)
		{
			m_cycleColours.push_back(CycleColor(strtol(&v["colour"].as<String>().c_str()[1], NULL, 16),
														v["duration"].as<unsigned int>(),
														v["durationtype"].as<String>().c_str(),
														v["beatdecay"].as<unsigned int>()));
		}
	}
	
	m_currentCycle = makeColourCycle(m_cycleColours.begin());

	reset(	RGB::hexToR(hexValue),
			RGB::hexToG(hexValue),
			RGB::hexToB(hexValue),
			power && (colour.length() > 0),
			(lightsetting == "beat") ? true : false);
}


void LEDRGB::notify(const Beatbox::Event& evt)
{
	setCurrentColourCycle(evt.beatDetected());
	if (evt.beatDetected())
	{
		m_lastPulse = millis();
		handle();
	}
}


void LEDRGB::handle()
{
	if (m_power && m_beatbox)
	{
		unsigned long now = millis();
		// use PWM to set the rgb strip values
#ifdef ESP32
		const RGB&		rgb		= getBeatColour();
		unsigned int	decay	= getDecay();

		const unsigned int light_multiplier = (now - m_lastPulse) > decay ? 0 : 1;
		
//		Serial.printf("RBG: %d, %d, %d.  Multiplier %d.  Decay %d\n", rgb.r(), rgb.g(), rgb.b(), light_multiplier, decay);

		ledcWrite(ledChannelR, rgb.r() * light_multiplier);
		ledcWrite(ledChannelG, rgb.g() * light_multiplier);
		ledcWrite(ledChannelB, rgb.b() * light_multiplier);
#elif defined(ESP8266)
		analogWrite(m_r_pin, rgb.r() - min((rgb.r() * (now - m_lastPulse)) / decay, static_cast<unsigned long>(rgb.r())));
		analogWrite(m_g_pin, rgb.g() - min((rgb.g() * (now - m_lastPulse)) / decay, static_cast<unsigned long>(rgb.g())));
		analogWrite(m_b_pin, rgb.b() - min((rgb.b() * (now - m_lastPulse)) / decay, static_cast<unsigned long>(rgb.b())));
#endif
		// Serial.printf("r decay, now, lastPulse, calc: %lu, %lu, %lu, %lu, %lu\n", m_r, decay, now, m_lastPulse, (m_r * now - m_lastPulse) / decay);
	}
}


void LEDRGB::reset(unsigned int r, unsigned int g, unsigned int b, bool power, bool beatbox)
{
	m_rgb.set_r(r);
	m_rgb.set_g(g);
	m_rgb.set_b(b);

	m_power = power;
	m_beatbox = beatbox;

	Serial.printf("RGB: %d, %d, %d, power %d, beatbox %d\n", m_rgb.r(), m_rgb.g(), m_rgb.b(), m_power, m_beatbox);

	if (m_power)
	{
		if (!m_beatbox)
		{
			// use PWM to set the rgb strip values
#ifdef ESP32
			ledcWrite(ledChannelR, m_rgb.r());
			ledcWrite(ledChannelG, m_rgb.g());
			ledcWrite(ledChannelB, m_rgb.b());
#elif defined(ESP8266)
			analogWrite(m_r_pin, m_r);
			analogWrite(m_g_pin, m_g);
			analogWrite(m_b_pin, m_b);
#endif
		}
	}
	else
	{
		// turn off the LEDs
#ifdef ESP32
		ledcWrite(ledChannelR, 0);
		ledcWrite(ledChannelG, 0);
		ledcWrite(ledChannelB, 0);
#elif defined(ESP8266)
		digitalWrite(m_r_pin, 0);
		digitalWrite(m_g_pin, 0);
		digitalWrite(m_b_pin, 0);
#endif
	}
}


const LEDRGB::RGB& LEDRGB::getBeatColour() const
{
	if (m_currentCycle.first != m_cycleColours.end())
		return m_currentCycle.first->rgb();
	return m_rgb;
}


unsigned int LEDRGB::getDecay() const
{
	if (m_currentCycle.first != m_cycleColours.end())
		return m_currentCycle.first->decay();
	return m_beatdecay;
}


const LEDRGB::CurrentCycle LEDRGB::makeColourCycle(CycleColors::const_iterator it) const
{

	unsigned long duration = 0;
	if (it != m_cycleColours.end())
		duration = (it->durationType() == CycleColor::DurationType::time) ? millis() : 1;

	return std::make_pair(it, duration);
}


void LEDRGB::setCurrentColourCycle(bool beatDetected)
{
	CycleColors::const_iterator itCurrent = m_currentCycle.first;
	if (itCurrent != m_cycleColours.end())
	{
		bool advance = false;

		if (itCurrent->durationType() == CycleColor::DurationType::beat)
		{
			if (beatDetected)
				advance = (++m_currentCycle.second > itCurrent->duration());
		}
		else
			advance = (millis() - m_currentCycle.second > itCurrent->duration());

		if (advance)
		{
			if (++itCurrent == m_cycleColours.end())
				itCurrent = m_cycleColours.begin();

			m_currentCycle = makeColourCycle(itCurrent);
		}
	}
}
