#pragma once

#include <MD_MAXPanel.h>
#include "Beatbox.h"


class PanelDisplay : public  SettingsListener, public SoundEvent::Listener
{
public:
	PanelDisplay(const uint8_t pin_data, const uint8_t pin_clk, const uint8_t pin_cs, const uint8_t devices_x, const uint8_t devices_y);

	virtual void notify(const JsonDocument& settings);
	virtual void notify(const SoundEvent& evt);

private:
	MD_MAXPanel		m_panel;
	String			m_message;
	bool			m_pulse			= false;
	unsigned int	m_scrollspeed	= 50;
	unsigned long	m_lastPulse		= 0;
	unsigned long	m_lastscroll	= 0;
	int				m_currentX		= -1;

	PanelDisplay(const PanelDisplay&);
	PanelDisplay& operator=(const PanelDisplay&) const;
};

