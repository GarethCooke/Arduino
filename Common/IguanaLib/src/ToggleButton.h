#pragma once

#include "ButtonStatus.h"

class ToggleButton
{
public:
	ToggleButton(ButtonStatus& button);

	bool consumeToggle();

private:
	ButtonStatus&				m_button;
	ButtonStatus::button_state	m_state;
};

