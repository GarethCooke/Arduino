#include <Arduino.h>
#include <algorithm>
#include <BeatWireReceiver.h>
#include <Wire.h>

using std::min;


BeatWireReceiver* BeatWireReceiver::m_pWireReceiver = NULL;


void BeatWireReceiver::create(TwoWire& wire, int sdaPin, int sclPin)
{
    static BeatWireReceiver m_receiver(wire, sdaPin, sclPin);
    m_pWireReceiver = &m_receiver;
}


BeatWireReceiver& BeatWireReceiver::get()
{
    return *m_pWireReceiver;
}


BeatWireReceiver::BeatWireReceiver(TwoWire& wire, int sdaPin, int sclPin) : BeatWireComms(wire)
{
    m_wire.onReceive(onReceive);
    m_wire.begin(m_address);
}


void BeatWireReceiver::onReceive(int bytes)
{
    m_pWireReceiver->receive(bytes);
}


void BeatWireReceiver::read(int bytes)
{
    m_wire.readBytes(m_buffer, min(bytes, BUF_SIZE));
}

void BeatWireReceiver::flush()
{
    int readNext = m_wire.available();
    while (readNext)
    {
        read(readNext);
        readNext = m_wire.available();
    }
}
