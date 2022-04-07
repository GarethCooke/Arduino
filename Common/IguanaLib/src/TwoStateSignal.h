#pragma once

class TwoStateSignal
{
public:
	virtual ~TwoStateSignal() {};
	virtual void signal(bool signalOn) = 0;

	void signalOn() { signal(true); }
	void signalOff() { signal(false); }
};


class DigitalPinSignal : public TwoStateSignal
{
public:
	DigitalPinSignal(unsigned int pin, bool highOn = true) : m_pin(pin), m_highOn(highOn)
	{
		pinMode(m_pin, OUTPUT);
		signalOff();
	}

	virtual void signal(bool signalOn) { digitalWrite(m_pin, signalOn == m_highOn ? HIGH : LOW); }

private:
	unsigned int	m_pin;
	bool			m_highOn;
};
