#pragma once

#include "IguanaBufStruct.h"

#define	LEN_SIGNATURE	14


class IguanaStored : public IguanaBufStruct
{
public:
	IguanaStored();

	bool isValid() const;

private:
	static const char* m_validSignature;

	char m_signature[LEN_SIGNATURE];
};

