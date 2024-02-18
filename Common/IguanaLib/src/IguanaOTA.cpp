#if defined(ESP8266) || (ESP32)

#include <ArduinoOTA.h>
#include "IguanaOTA.h"

std::auto_ptr<IguanaOTA> m_pSingleton;

bool IguanaOTA::Initialise(const char *szHostname)
{
	bool bSuccess = !m_pSingleton.get();

	if (bSuccess)
	{
		m_pSingleton.reset(new IguanaOTA(szHostname));
		bSuccess = m_pSingleton.get();
	}

	return bSuccess;
}

IguanaOTA::IguanaOTA(const char *szHostname)
{
	// Port defaults to 8266
	// ArduinoOTA.setPort(8266);

	// Hostname defaults to esp8266-[ChipID]
	ArduinoOTA.setHostname(szHostname);

	// No authentication by default
	ArduinoOTA.setPassword("password");

	ArduinoOTA.onStart([]()
					   { Serial.println("Start"); });
	ArduinoOTA.onEnd([]()
					 { Serial.println("\nEnd"); });
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
						  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
	ArduinoOTA.onError([](ota_error_t error)
					   {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed"); });
	ArduinoOTA.begin();
}

void IguanaOTA::handle()
{
	ArduinoOTA.handle();
}

#endif