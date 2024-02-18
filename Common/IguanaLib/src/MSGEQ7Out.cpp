#include <Arduino.h>
#include "MSGEQ7Out.h"

const char* MSGEQ7Out::m_frequencies[BANDS] = { "63Hz", "160Hz", "400Hz", "1000Hz", "2500Hz", "6250Hz", "16000Hz" };

void MSGEQ7Out::fromyBytes(const uint8_t* src)
{
    memcpy(this, src, bytes());
}
