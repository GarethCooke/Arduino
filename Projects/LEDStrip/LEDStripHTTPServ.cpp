#include <Arduino.h>
#include <SPIFFS.h>
#include <algorithm>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "LEDStripHTTPServ.h"
#include "LEDStripController.h"

#define MAX_SSID_NAME 128

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
std::auto_ptr<AsyncWebServer> pServer;
AsyncEventSource events("/events");


LEDStripHTTPServ::LEDStripHTTPServ(LEDStripController& controller) : m_controller(controller), m_performReboot(0)
{
	if(!pServer.get());
	{
		pServer.reset(new AsyncWebServer(80));

		Serve(*pServer, "/styles.css",						"text/css");
		Serve(*pServer, "/main.js",							"text/javascript");
		Serve(*pServer, "/polyfills.js",					"text/javascript");
		Serve(*pServer, "/runtime.js",						"text/javascript");
		Serve(*pServer, m_controller.settingsPath(),		"text/json", m_controller.settingsFilename());
		Serve(*pServer, "/",	"",							"/index.html");
		Serve(*pServer, "/index.html");
		Serve(*pServer, "/favicon.ico");
		Serve(*pServer, "/DOTMATRI.TTF");
		Serve(*pServer, "/DOTMBold.TTF");

		// Route to store JSON settings data file
		pServer->on(m_controller.settingsPath(), HTTP_POST, [](AsyncWebServerRequest* request)
		{ request->send(200, "text/plain"); }, NULL,
			[this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) { handleSettingsChange(data, len); });

		pServer->on("/api/networkdata", HTTP_GET, [this](AsyncWebServerRequest* request) {
			request->send(200, "text/json", *getNetworks());
		});

		pServer->on("/api/networkset", HTTP_POST, [](AsyncWebServerRequest* request)
		{ request->send(200, "text/plain", "Post route"); }, NULL,
			[this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) { handleNetworkChange(data, len); });

		events.onConnect([](AsyncEventSourceClient* client) {
			Serial.println("onconnect");
			if (client->lastId()) {
				Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
			}

			// send sample event message
			// and set reconnect delay to 1 second
			//client->send("{\"msg\":{\"frequencies\":[{\"band\":\"63Hz\",\"value\":15},{\"band\":\"160Hz\",\"value\":26},{\"band\":\"400Hz\",\"value\":18},{\"band\":\"1000Hz\",\"value\":97},{\"band\":\"2500Hz\",\"value\":57},{\"band\":\"6250Hz\",\"value\":83},{\"band\":\"16000Hz\",\"value\":10}],\"beatDetected\":false}}", NULL, millis(), 1000);
		});

//		events.setAuthentication("user", "pass");
		pServer->addHandler(&events);

		Serve(*pServer, "*", "", "/index.html");  // required because we have an angular client

		pServer->begin();
		Serial.println("HTTP server started");

		getNetworks();
	}
}


void LEDStripHTTPServ::Serve(AsyncWebServer& server, const char* filename, const char* data_type, const char* sourcefile)
{
	pServer->on(filename, HTTP_GET, [filename, data_type, sourcefile](AsyncWebServerRequest* request) {
		request->send(SPIFFS, sourcefile ? sourcefile : filename, data_type);
	});
}


void LEDStripHTTPServ::handleSettingsChange(uint8_t* data, size_t len)
{
	File f = SPIFFS.open(m_controller.settingsFilename(), "w");
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
		String SSID	= doc["SSID"];
		String pwd	= doc["pwd"];
		m_controller.resetWiFiInfo(SSID, pwd);
	}
}


// First request will return 0 results unless you start scan from somewhere else (loop/setup)
// Do not request more often than 3-5 seconds
std::auto_ptr<String> LEDStripHTTPServ::getNetworks()
{
	std::auto_ptr<String> pString(new String());
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

		const size_t capacity = JSON_ARRAY_SIZE(networkCount + 1) + (networkCount + 2) * (JSON_OBJECT_SIZE(1) + MAX_SSID_NAME + 64);
		DynamicJsonDocument jsonDoc(capacity);

		JsonObject root		= jsonDoc.to<JsonObject>();
		root["selected"]	= m_controller.getWiFiConfig().getSSID();
		root["connected"]	= m_controller.isWifiConnected() ? "true" : "false";

		JsonArray nets = root.createNestedArray("networks");

		pString.reset(new String());

		for (unsigned int nNetwork = 0; nNetwork < networkCount; ++nNetwork)
		{
			JsonObject network = nets.createNestedObject();
			network["name"]		= WiFi.SSID(nNetwork);
			network["strength"]	= 2 * (WiFi.RSSI(nNetwork) + 100);
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


void LEDStripHTTPServ::notify(const Beatbox::Event& evt)
{
	static unsigned long prevMillis = millis();
	unsigned long nowMillis = millis();
	if ((nowMillis - prevMillis > 200) || evt.beatDetected())
	{
		prevMillis = nowMillis;
		const size_t capacity = JSON_ARRAY_SIZE(5) + 3 * (JSON_OBJECT_SIZE(1) + MAX_SSID_NAME + 64);
		DynamicJsonDocument jsonDoc(capacity);

		JsonObject root			= jsonDoc.to<JsonObject>();
		JsonObject msg			= root.createNestedObject("msg");
		JsonArray frequencies	= msg.createNestedArray("frequencies");

		std::for_each(evt.begin(), evt.end(), [frequencies](const std::pair<const String*, int>& band)
		{
			JsonObject jsonBand = frequencies.createNestedObject();
			jsonBand["band"] = *band.first;
			jsonBand["value"] = band.second;
		});

		msg["beatDetected"] = evt.beatDetected();

		String strJSON;
		serializeJson(jsonDoc, strJSON);
		events.send(strJSON.c_str());
	}
}
