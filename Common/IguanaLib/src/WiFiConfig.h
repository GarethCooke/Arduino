#pragma once

#pragma pack(push)  // push current alignment to stack
#pragma pack(1)     // set alignment to 1 byte boundary

#include "IguanaStored.h"

#define LEN_SSID	33
#define LEN_PWD		65


class WiFiConfig : public IguanaStored
{
public:
	WiFiConfig();
	WiFiConfig(const char* szSSID, const char* szPwd);

	bool		hasSSID()		const { return strlen(m_ssid);	}
	const char*	getSSID()		const { return m_ssid;			}
	const char*	getPassword()	const { return m_pwd;			}

	void reset();
	bool setSSID(const char* ssid);
	bool setPassword(const char* pwd);

private:
	char m_ssid[LEN_SSID];
	char m_pwd[LEN_PWD];
};

#pragma pack(pop)