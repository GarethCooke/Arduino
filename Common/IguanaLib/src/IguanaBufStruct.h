#pragma once
class IguanaBufStruct
{
public:
	IguanaBufStruct();
	~IguanaBufStruct();

protected:
	static bool checkParam(char* dest, const char* paramName, const char* value, unsigned int allowedLength);
};

