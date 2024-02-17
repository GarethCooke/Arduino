#pragma once

class TwoStateValue
{
public:
	virtual ~TwoStateValue() {};
	virtual bool isSet() = 0;
};


class DigitalPinValue : public TwoStateValue
{
public:
	DigitalPinValue(unsigned int pin, bool highOn = true) : m_pin(pin), m_highOn(highOn)
	{
		pinMode(m_pin, INPUT);
	}

	virtual bool isSet() { return digitalRead(m_pin) ^ !m_highOn; }

private:
	unsigned int	m_pin;
	bool			m_highOn;
};
