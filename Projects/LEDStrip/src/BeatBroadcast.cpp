#include <Wire.h>
#include "BeatBroadcast.h"

BeatBroadcast::BeatBroadcast(TwoWire &wire) : m_wire(wire)
{
}

void BeatBroadcast::notify(const SoundEvent::Output &evt)
{
    m_wire.beginTransmission(m_address);
    Serial.print("Write result: ");
    Serial.println(m_wire.write(0x45));
    m_wire.endTransmission();
}
