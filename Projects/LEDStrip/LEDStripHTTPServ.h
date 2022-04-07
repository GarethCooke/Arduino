#pragma once

#include <vector>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include "Beatbox.h"


class LEDStripHTTPServ : public Beatbox::EventListener
{
public:
	LEDStripHTTPServ(LEDStripController& ledstrip);

	virtual void notify(const Beatbox::Event& evt);

private:
	class NetworkInfo
	{
	public:
		NetworkInfo(const String& ssid, const int32_t rssi, bool isEncrypted)
			: m_SSID(ssid), m_RSSI(rssi), m_isEncrypted(isEncrypted) {}

		const String&	getSSID()		const { return m_SSID;			}
		int64_t			getRSSI()		const { return m_RSSI;			}
		bool			isEncrypted()	const { return m_isEncrypted;	}

	private:
		const String	m_SSID;
		const int32_t	m_RSSI;
		const bool		m_isEncrypted;
	};

	LEDStripController&	m_controller;
	unsigned long		m_performReboot;

	void Serve(AsyncWebServer& server, const char* filename, const char* data_type = "", const char* sourcefile = NULL);
	void handleSettingsChange(uint8_t* data, size_t len);
	void handleNetworkChange(uint8_t* data, size_t len);
	std::auto_ptr<String> getNetworks();
};
