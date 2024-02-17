#include <Wire.h>
#include "BeatReceiver.h"

BeatReceiver::BeatReceiver(TwoWire &wire, int sdaPin, int sclPin) : m_wire(wire)
{
    m_wire.onReceive(receive);
    m_wire.begin(sdaPin, sclPin);
}

BeatReceiver::~BeatReceiver()
{
}

void BeatReceiver::receive(int bytes)
{
}
