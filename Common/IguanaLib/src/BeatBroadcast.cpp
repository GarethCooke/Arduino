#include <Arduino.h>
#include <Wire.h>
#include "BeatBroadcast.h"

BeatBroadcast::BeatBroadcast(TwoWire& wire) : m_wire(wire)
{
}

BeatBroadcast::~BeatBroadcast()
{
}

void BeatBroadcast::notify(const MSGEQ7Out& evt)
{
    m_wire.beginTransmission(m_address);
    m_wire.write(evt.toBytes(), evt.bytes());
    m_wire.endTransmission();
}
