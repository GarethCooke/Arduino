#pragma once
class NetworkHost
{
public:
	virtual const char* getHostName()	const { return "N/A"; }
	virtual String getMACAddress()		const { return "N/A"; }
	virtual String getIPAddress()		const { return "N/A"; }
};
