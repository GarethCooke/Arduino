#include <Arduino.h>
#include "ButtonStatus.h"



ButtonStatus::ButtonStatus(shared_ptr_lite<TwoStateValue> pBtn) : m_pBtn(pBtn.release()), m_lastDebounceTime(millis())
{
	m_lastButtonState = m_buttonState = m_pBtn->isSet() ? on : off;
}


ButtonStatus::~ButtonStatus()
{
	delete m_pBtn;
}


const ButtonStatus::button_state ButtonStatus::getState()
{
	const bool reading		= m_pBtn->isSet();
	const unsigned long now	= millis();

	// check to see if you just pressed the button
	// (i.e. the input went from LOW to HIGH),  and you've waited
	// long enough since the last press to ignore any noise:

	// If the switch changed, due to noise or pressing:
	if (reading != m_lastButtonState)
	{
		m_lastDebounceTime	= now; // reset the debouncing timer
		m_lastButtonState	= reading;
		m_buttonState		= unknown;
	}

	if ((now - m_lastDebounceTime) > m_debounceDelay)
	{
		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state:

		m_buttonState = reading ? on : off;
	}

	return m_buttonState;
}
