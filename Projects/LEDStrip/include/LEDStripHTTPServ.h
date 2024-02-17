#pragma once

#include <memory>
#include <semphr.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include "Beatbox.h"
#include "SoundEvent.h"

using std::unique_ptr;

class LEDStripController;
class AsyncWebServer;
class AsyncEventSource;

class LEDStripHTTPServ : public SoundEvent::Listener
{
public:
	virtual ~LEDStripHTTPServ();

	static void create(LEDStripController &controller);
	static LEDStripHTTPServ &get();

	virtual void notify(const SoundEvent::Output &evt);

	void handle();

private:
	class NetworkInfo
	{
	public:
		NetworkInfo(const String &ssid, const int32_t rssi, bool isEncrypted)
			: m_SSID(ssid), m_RSSI(rssi), m_isEncrypted(isEncrypted) {}

		const String &getSSID() const { return m_SSID; }
		int64_t getRSSI() const { return m_RSSI; }

	private:
		const String m_SSID;
		const int32_t m_RSSI;
		const bool m_isEncrypted;
	};

	LEDStripHTTPServ(LEDStripController &ledstrip);

	LEDStripController &m_controller;
	unsigned long m_performReboot;
	AsyncWebServer *m_pServer;
	AsyncEventSource *m_pEvents;
	QueueHandle_t m_queue;

	void Serve(AsyncWebServer &server, const char *filename, const char *data_type = "", const char *sourcefile = NULL);
	void handleSettingsChange(uint8_t *data, size_t len);
	void handleNetworkChange(uint8_t *data, size_t len);
	std::unique_ptr<String> getNetworks();
	void notifySubsctibers(const SoundEvent::Output &evt);
};
