#include <Wire.h>
#include "BeatWireSender.h"

void BeatWireSender::send(const MSGEQ7Out& evt)
{
    m_wire.beginTransmission(m_address);
    m_wire.write(evt.toBytes(), evt.bytes());
    m_wire.endTransmission();
}