#include <Arduino.h>
#include <string.h>
#include "IguanaStored.h"

const char* IguanaStored::m_validSignature = "Signature";


IguanaStored::IguanaStored()
{
	checkParam(m_signature, m_validSignature, m_validSignature, LEN_SIGNATURE);
}


bool IguanaStored::isValid() const
{
	return !strncmp(m_signature, m_validSignature, LEN_SIGNATURE);
}
