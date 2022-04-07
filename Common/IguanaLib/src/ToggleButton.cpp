#include <Arduino.h>
#include "ToggleButton.h"


ToggleButton::ToggleButton(ButtonStatus& button) : m_button(button), m_state(button.getState())
{
}


bool ToggleButton::consumeToggle()
{
	bool toggle = (m_button.getState() != m_state) && m_button.isStateKnown() && ButtonStatus::isKnownState(m_state);
	if (toggle)
	{
		m_state = m_button.getState();
	}
	return toggle && ButtonStatus::isOn(m_state);
}