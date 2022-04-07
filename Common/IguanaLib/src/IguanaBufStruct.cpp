#include <Arduino.h>
#include "IguanaBufStruct.h"



IguanaBufStruct::IguanaBufStruct()
{
}


IguanaBufStruct::~IguanaBufStruct()
{
}


bool IguanaBufStruct::checkParam(char* dest, const char* paramName, const char* value, unsigned int allowedLength)
{
	bool success = value && (strlen(value) < allowedLength);
	if (!success)
	{
		Serial.print("Parameter part ");
		Serial.print(paramName);
		Serial.print(" invalid.  ");
		if (!value)
			Serial.println("Pointer is NULL.");
		else
		{
			Serial.print("Sting length of ");
			Serial.print(strlen(value));
			Serial.print(" is greater than allowed strning length of ");
			Serial.print(allowedLength - 1);
			Serial.print(".  String is \"");
			Serial.print(value);
			Serial.println("\".");
		}
	}
	else
		strcpy(dest, value);

	return success;
}
