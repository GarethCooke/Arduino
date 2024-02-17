#include <Arduino.h>
#include <Wire.h>
#include "BeatBroadcast.h"

BeatBroadcast::BeatBroadcast(TwoWire &wire) : m_wire(wire)
{
    m_wire.beginTransmission(m_address);
}

BeatBroadcast::~BeatBroadcast()
{
    m_wire.endTransmission();
}

void BeatBroadcast::notify(const MSGEQ7Out &evt)
{
    Serial.print("Write result: ");
    Serial.println(m_wire.write("Test transmission"));
}
