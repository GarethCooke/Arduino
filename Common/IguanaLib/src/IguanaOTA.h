#pragma once

class IguanaOTA
{
public:
	static bool Initialise(const char* szHostname);
	static void handle();

private:
	IguanaOTA(const char* szHostname);
};

