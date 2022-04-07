#include <string.h>
#include "WiFiConfig.h"


WiFiConfig::WiFiConfig()
{
	memset(m_ssid, 0, sizeof(m_ssid));
	memset(m_pwd, 0, sizeof(m_ssid));
}


WiFiConfig::WiFiConfig(const char* szSSID, const char* szPwd)
{
	setSSID(szSSID);
	setPassword(szPwd);
}


void WiFiConfig::reset()
{
	*this = WiFiConfig();
}


bool WiFiConfig::setSSID(const char* ssid)
{
	return checkParam(m_ssid, "SSID", ssid, LEN_SSID);
}


bool WiFiConfig::setPassword(const char* pwd)
{
	return checkParam(m_pwd, "Password", pwd, LEN_PWD);
}
