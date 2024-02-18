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


void BeatWireReceiver::receive(int bytes)
{
    MSGEQ7Out event;
    static uint8_t buffer[255];

    m_wire.readBytes(buffer, min(bytes, static_cast<int>(sizeof(buffer))));
    if (bytes == event.bytes())
    {
        memcpy(&event, buffer, bytes);
        BeatWireReceiver& receiver = BeatWireReceiver::get();
        if (receiver.m_pReceiver)
            receiver.m_pReceiver->received(event);
    }
    else
    {
        // dont recognise this message - read and discard bytes to allow other message processing
        int readNext = Wire.available();
        while (readNext)
        {
            m_wire.readBytes(buffer, min(readNext, static_cast<int>(sizeof(buffer))));
            readNext = m_wire.available();
        }
    }
}
