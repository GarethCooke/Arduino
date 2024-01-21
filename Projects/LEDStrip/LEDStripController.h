#pragma once

#include <set>
#include <memory>
#include <map>
#include <ctime>
#include <WiFiConfig.h>
#include <ArduinoJson.h>
#include "NetworkHost.h"


class LEDStripController : public NetworkHost
{
public:
	LEDStripController();
	~LEDStripController();

	class SettingsListener
	{
	public:
		virtual void notify(const JsonDocument& settings) = 0;
	};
	void addListener(SettingsListener* pListener);
	void removeListener(SettingsListener* pListener);

	void factory_reset();

	virtual const char* getHostName()	const { return m_device_hostname; }
	virtual String getMACAddress()		const;
	virtual String getIPAddress()		const;

	void resetWiFiInfo(const String& strSSID, const String& strPassword, bool resetNow = true);
	void reset();
	void resetFromSettings();
	const char* settingsPath() const { return "/api/settings"; }
	const char* settingsFilename() const { return "/api/settings.json"; }
	const WiFiConfig& getWiFiConfig() const { return m_wificonfig; }
	bool isWifiConnected() const { return m_wifiConnected; }

private:
	static const char*			m_device_hostname;
	std::time_t					m_startTime;
	WiFiConfig					m_wificonfig;
	bool						m_wifiConnected;
	std::set<SettingsListener*>	m_listeners;

	LEDStripController(const LEDStripController&);
	LEDStripController& operator=(const LEDStripController&);

	void setupWiFi();
	bool startupWiFi(const char* ssid, const char* pwd);
};
