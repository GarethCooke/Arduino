#pragma once

#include <ArduinoJson.h>

class SettingsListener
{
public:
	virtual void notify(const JsonDocument& settings) = 0;
};
