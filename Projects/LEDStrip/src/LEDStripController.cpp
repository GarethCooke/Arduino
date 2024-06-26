#include <Arduino.h>
#include <WiFi.h>
#include <sstream>
#include <iomanip>
#include <time.h>
#include <algorithm>
#include <functional>
#include <EEPROM.h>
#include <LittleFS.h> 
#include "LEDStripController.h"

#define EEPROM_WiFiConfig	0

const char* LEDStripController::m_device_hostname = "LEDStrip";

const uint64_t	rAddressPair = 0xE8E8F0F0E1LL;  // pipe to recive data on
const uint64_t	wAddressPair = 0xB00B1E50B1LL;  // pipe to write or transmit on
uint64_t		rAddressMain = 0;				// pipe to recive data on


LEDStripController::LEDStripController()
	: m_wifiConnected(false)
{
	if (!LittleFS.begin())
	{
		Serial.println("An Error has occurred while mounting LittleFS");
		return;
	}

	EEPROM.begin(sizeof(WiFiConfig));
	EEPROM.get(EEPROM_WiFiConfig, m_wificonfig);

	if (!m_wificonfig.isValid())
		m_wificonfig.reset();

	Serial.print("Is config valid: ");
	Serial.println(m_wificonfig.isValid() ? "TRUE" : "FALSE");

	Serial.print("SSID: ");
	Serial.println(m_wificonfig.getSSID());
	Serial.print("PWD: ");
	Serial.println(strlen(m_wificonfig.getPassword()) ? "************" : "");

	if (m_wificonfig.isValid() && m_wificonfig.hasSSID())
		m_wifiConnected = startupWiFi(m_wificonfig.getSSID(), m_wificonfig.getPassword()); // to connect using stored connection parameters

	if (!m_wifiConnected)
		setupWiFi(); // so setup as an access point

	Serial.print("MAC Address is ");
	Serial.println(WiFi.macAddress());

	byte mac[8];
	WiFi.macAddress(mac);

	rAddressMain = *((uint64_t*)mac);

	// initialise the lights
	resetFromSettings();

	Serial.println("setup complete");
}


LEDStripController::~LEDStripController()
{
	LittleFS.end();
}


void LEDStripController::addListener(SettingsListener* pListener)
{
	m_listeners.insert(pListener);
}


void LEDStripController::removeListener(SettingsListener* pListener)
{
	m_listeners.erase(pListener);
}


void LEDStripController::factory_reset()
{
	Serial.println("Reset to factory settings.");
	m_wificonfig = WiFiConfig();
	EEPROM.put(EEPROM_WiFiConfig, m_wificonfig);
	EEPROM.commit();
	reset();
}


String LEDStripController::getMACAddress() const
{
	return WiFi.macAddress();
}


String LEDStripController::getIPAddress() const
{
	return WiFi.localIP().toString();
}


void LEDStripController::reset()
{
	Serial.println("Reboot");
	delay(1000);
	ESP.restart();
}


void LEDStripController::resetWiFiInfo(const String& strSSID, const String& strPassword, bool resetNow)
{
	if (m_wificonfig.setSSID(strSSID.c_str()) && m_wificonfig.setPassword(strPassword.c_str()))
	{
		Serial.print("SSID: ");
		Serial.println(strSSID);
		Serial.print("PWD: ");
		Serial.println(strPassword);

		Serial.print("Is wifi config valid: ");
		Serial.println(m_wificonfig.isValid());
		Serial.print("SSID: ");
		Serial.println(m_wificonfig.getSSID());
		Serial.print("PWD: ");
		Serial.println(m_wificonfig.getPassword());

		EEPROM.put(EEPROM_WiFiConfig, m_wificonfig);
		EEPROM.commit();

		if (resetNow)
			reset();
	}
}


void LEDStripController::resetFromSettings()
{
	File f = LittleFS.open(settingsFilename(), "r");

	Serial.printf("Settings file size %d\n", f.size());

	DynamicJsonDocument doc(f.size() * 3);
	// Deserialize the JSON document
	DeserializationError error = deserializeJson(doc, f);

	// Close the file
	f.close();

	if (error)
		Serial.println(F("Failed to read settings file"));
	else
		for_each(m_listeners.begin(), m_listeners.end(), [doc](std::set<SettingsListener*>::const_reference nextListener) { nextListener->notify(doc); });
}


void LEDStripController::setupWiFi()
{
	WiFi.disconnect(true, true);
	WiFi.mode(WIFI_AP);
	WiFi.softAP(m_device_hostname);

	Serial.println("Wait 100 ms for AP_START...");
	delay(100);

	Serial.println("Setting the AP");
	Serial.println("Set softAPConfig");
	IPAddress Ip(192, 168, 1, 1);
	IPAddress NMask(255, 255, 255, 0);
	WiFi.softAPConfig(Ip, Ip, NMask);

	Serial.print("Server IP address: ");
	Serial.println(WiFi.softAPIP());
}


bool LEDStripController::startupWiFi(const char* ssid, const char* pwd)
{
	Serial.println("Starting WiFi");
	WiFi.mode(WIFI_STA);

	WiFi.setHostname(m_device_hostname);
	WiFi.begin(ssid, pwd);

	bool success = (WiFi.waitForConnectResult() == WL_CONNECTED);
	if (success)
	{
		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());
	}

	return success;
}
