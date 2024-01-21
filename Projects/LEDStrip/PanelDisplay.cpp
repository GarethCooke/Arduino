#include "PanelDisplay.h"
#include "Font5x3.h"


PanelDisplay::PanelDisplay(const uint8_t pin_data, const uint8_t pin_clk, const uint8_t pin_cs, const uint8_t devices_x, const uint8_t devices_y)
	: m_panel(MD_MAX72XX::FC16_HW, pin_data, pin_clk, pin_cs, devices_x, devices_y)
{
	m_panel.begin();
	m_panel.setFont(_Fixed_5x3);
	m_panel.setIntensity(4);
	m_currentX = m_panel.getXMax();
}


void PanelDisplay::notify(const JsonDocument& settings)
{
	m_message		= settings["message"].as<String>();
	m_pulse			= settings["messagescroll"] != true;
	m_scrollspeed	= settings["scrollspeed"].as<unsigned int>();
}


void PanelDisplay::notify(const SoundEvent& evt)
{
	unsigned long now = millis();
	uint16_t USER_MESG = m_panel.getFontHeight() + 1;

	bool pulseClear	= (now - m_lastPulse > 200);
	bool newBeat = evt.beatDetected();

	if (newBeat)
		m_lastPulse = now;

	if (m_pulse)
	{
		if (pulseClear)
			m_panel.clear(0, 0, m_panel.getXMax(), m_panel.getYMax());

		if (newBeat)
		{
			uint16_t end = m_panel.drawText(1, USER_MESG, m_message.c_str());
			m_panel.drawFillRectangle(end + 2, 3, m_panel.getXMax() - 1, 4);
		}
	}
	else
	{
		bool refresh = false;
		if (!m_scrollspeed)
		{
			if (refresh = m_currentX != 0)
				m_currentX = 0;
		}
		else if (refresh = (now - m_lastscroll) > (1000 / m_scrollspeed))
		{
			m_lastscroll = now;
			m_currentX--;
		}

		if (refresh)
		{
			m_panel.clear(0, 0, m_panel.getXMax(), m_panel.getYMax());
			uint16_t end = m_panel.drawText(m_currentX, USER_MESG, m_message.c_str());
			if(m_currentX + end == 0)
				m_currentX = m_panel.getXMax();
		}

		// set beat pixel
		m_panel.setPoint(m_panel.getXMax(), m_panel.getYMax(), !pulseClear || newBeat);
	}
}
