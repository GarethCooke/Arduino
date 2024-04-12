#include <Arduino.h>
#include <LittleFS.h>
#include <algorithm>
#include <stdexcept>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "LEDStripHTTPServ.h"
#include "LEDStripController.h"

using std::logic_error;
using std::runtime_error;

#define MAX_SSID_NAME 128

std::unique_ptr<LEDStripHTTPServ> pHTTPServer;

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
void LEDStripHTTPServ::create(LEDStripController& controller)
{
	if (pHTTPServer.get())
		throw logic_error("Attempt to create more than oneLEDStripHTTPServ instances.");

	pHTTPServer.reset(new LEDStripHTTPServ(controller));
}

LEDStripHTTPServ& LEDStripHTTPServ::get()
{
	if (!pHTTPServer.get())
		throw logic_error("Attempt to get() LEDStripHTTPServ before call to create(...).");
	return *pHTTPServer;
}

LEDStripHTTPServ::LEDStripHTTPServ(LEDStripController& controller)
	: m_controller(controller), m_performReboot(0), m_pServer(new AsyncWebServer(80)), m_pEvents(new AsyncEventSource("/events")), m_queue(xQueueCreate(2, sizeof(MSGEQ7Out)))
{
	Serve(*m_pServer, "^/.+-.+[.]css$", "text/css");
	Serve(*m_pServer, "^/.+-.+[.]js$", "text/javascript");
	Serve(*m_pServer, m_controller.settingsPath(), "text/json", m_controller.settingsFilename());
	Serve(*m_pServer, "/", "", "/index.html");
	Serve(*m_pServer, "/index.html");
	Serve(*m_pServer, "^/.+[.]ico$");
	Serve(*m_pServer, "^/.+[.]TTF$");

	// Route to store JSON settings data file
	m_pServer->on(
		m_controller.settingsPath(), HTTP_POST, [](AsyncWebServerRequest* request)
		{ request->send(200, "text/plain"); },
		NULL,
		[this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total)
		{ handleSettingsChange(data, len); });

	m_pServer->on("/api/networkdata", HTTP_GET, [this](AsyncWebServerRequest* request)
		{ request->send(200, "text/json", *getNetworks()); });

	m_pServer->on(
		"/api/networkset", HTTP_POST, [](AsyncWebServerRequest* request)
		{ request->send(200, "text/plain", "Post route"); },
		NULL,
		[this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total)
		{ handleNetworkChange(data, len); });

	m_pEvents->onConnect([](AsyncEventSourceClient* client)
		{
			Serial.println("onconnect");
			if (client->lastId())
			{
				Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
			}

			// send sample event message
			// and set reconnect delay to 1 second
			// client->send("{\"msg\":{\"frequencies\":[{\"band\":\"63Hz\",\"value\":15},{\"band\":\"160Hz\",\"value\":26},{\"band\":\"400Hz\",\"value\":18},{\"band\":\"1000Hz\",\"value\":97},{\"band\":\"2500Hz\",\"value\":57},{\"band\":\"6250Hz\",\"value\":83},{\"band\":\"16000Hz\",\"value\":10}],\"beatDetected\":false}}", NULL, millis(), 1000);
		});

	//		m_pEvents->setAuthentication("user", "pass");
	m_pServer->addHandler(m_pEvents);

	Serve(*m_pServer, "*", "", "/index.html"); // required because we have an angular client

	m_pServer->begin();
	Serial.println("HTTP server started");

	getNetworks();
}

LEDStripHTTPServ::~LEDStripHTTPServ()
{
	delete m_pServer;
	delete m_pEvents;
}

void LEDStripHTTPServ::Serve(AsyncWebServer& server, const char* filename, const char* data_type, const char* sourcefile)
{
	m_pServer->on(filename, HTTP_GET, [filename, data_type, sourcefile](AsyncWebServerRequest* request) { request->send(LittleFS, sourcefile ? sourcefile : request->url().c_str(), data_type); });
}

void LEDStripHTTPServ::handleSettingsChange(uint8_t* data, size_t len)
{
	File f = LittleFS.open(m_controller.settingsFilename(), "w");
	if (f)
	{
		Serial.println("writing...");
		size_t byteswritten = f.write(data, len);
		Serial.printf("%d bytes written\n", byteswritten);
		f.close();
	}
	m_controller.resetFromSettings();
}

void LEDStripHTTPServ::handleNetworkChange(uint8_t* data, size_t len)
{
	Serial.println(F("Changing network settings"));
	DynamicJsonDocument doc(len * 3);
	// Deserialize the JSON document
	DeserializationError error = deserializeJson(doc, data, len);
	if (error)
		Serial.println(F("Failed to read network settings file"));
	else
	{
		String SSID = doc["SSID"];
		String pwd = doc["pwd"];
		m_controller.resetWiFiInfo(SSID, pwd);
	}
}

// First request will return 0 results unless you start scan from somewhere else (loop/setup)
// Do not request more often than 3-5 seconds
std::unique_ptr<String> LEDStripHTTPServ::getNetworks()
{
	std::unique_ptr<String> pString(new String());
	Serial.println("scan started");
	int networkCount = WiFi.scanComplete();
	Serial.println("scan complete");
	if (networkCount == -2)
	{
		Serial.println("scan started");
		WiFi.scanNetworks(true);
	}
	else
	{
		Serial.print(networkCount);
		Serial.println(" networks found");

		JsonDocument jsonDoc;

		JsonObject root = jsonDoc.to<JsonObject>();
		root["selected"] = m_controller.getWiFiConfig().getSSID();
		root["connected"] = m_controller.isWifiConnected() ? "true" : "false";

		JsonArray nets = root.createNestedArray("networks");

		pString.reset(new String());

		for (unsigned int nNetwork = 0; nNetwork < networkCount; ++nNetwork)
		{
			JsonObject network = nets.createNestedObject();
			network["name"] = WiFi.SSID(nNetwork);
			network["strength"] = 2 * (WiFi.RSSI(nNetwork) + 100);
			network["secured"] = WiFi.encryptionType(nNetwork) != WIFI_AUTH_OPEN;
		}

		serializeJson(jsonDoc, *pString);

		WiFi.scanDelete();
		if (WiFi.scanComplete() == -2)
			WiFi.scanNetworks(true);

		Serial.printf("%d networks detected\n", networkCount);
	}
	Serial.println(pString->c_str());
	return pString;
}

void LEDStripHTTPServ::notify(const MSGEQ7Out& evt)
{
	xQueueSendToBack(m_queue, &evt, 0);
}

void LEDStripHTTPServ::notifySubsctibers(const MSGEQ7Out& evt)
{
	static unsigned long prevMillis = millis();
	unsigned long nowMillis = millis();
	if ((nowMillis - prevMillis > 200) || evt.beatDetected())
	{
		prevMillis = nowMillis;
		const size_t capacity = JSON_ARRAY_SIZE(5) + 3 * (JSON_OBJECT_SIZE(1) + MAX_SSID_NAME + 64);
		DynamicJsonDocument jsonDoc(capacity);

		JsonObject root = jsonDoc.to<JsonObject>();
		JsonObject msg = root.createNestedObject("msg");
		JsonArray frequencies = msg.createNestedArray("frequencies");

		evt.iterate_bands([frequencies](const char* frequency, unsigned int value, bool beat)
			{
				JsonObject jsonBand = frequencies.createNestedObject();
				jsonBand["band"] = frequency;
				jsonBand["value"] = value; });

		msg["beatDetected"] = evt.beatDetected();

		String strJSON;
		serializeJson(jsonDoc, strJSON);
		m_pEvents->send(strJSON.c_str());
	}
}

void LEDStripHTTPServ::handle()
{
	MSGEQ7Out evt;
	while (xQueueReceive(m_queue, &evt, 0) == pdTRUE)
		notifySubsctibers(evt);
}
