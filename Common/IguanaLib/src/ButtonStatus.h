#pragma once

#include <shared_ptr_lite.h>
#include <TwoStateValue.h>

class ButtonStatus
{
public:
	ButtonStatus(shared_ptr_lite<TwoStateValue> pBtn);
	~ButtonStatus();

	enum button_state
	{
		off,
		on,
		unknown
	};

	const button_state getState();
	static bool isKnownState(button_state btnState)	{ return btnState != unknown;			}
	bool isOn()										{ return isOn(getState());				}
	bool isOff()									{ return isOff(getState());				}
	static bool isOn(button_state btnState)			{ return btnState == on;				}
	static bool isOff(button_state btnState)		{ return btnState == off;				}
	bool isStateKnown() const						{ return isKnownState(m_buttonState);	}

private:
	static const long	m_debounceDelay = 50;
	TwoStateValue*		m_pBtn;
	button_state		m_buttonState;
	int					m_lastButtonState;
	unsigned long		m_lastDebounceTime;

	ButtonStatus(const ButtonStatus&);
	ButtonStatus& operator=(const ButtonStatus&);
};