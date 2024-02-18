#include <Arduino.h>
#include <memory>
#include <algorithm>
#include <Wire.h>
#include "BeatReceiver.h"

using std::for_each;
using std::min;

static BeatReceiver* pReceiver = NULL;


void BeatReceiver::create(TwoWire& wire, int sdaPin, int sclPin)
{
    static BeatReceiver m_receiver(wire, sdaPin, sclPin);
    pReceiver = &m_receiver;
}

BeatReceiver& BeatReceiver::get()
{
    return *pReceiver;
}

BeatReceiver::BeatReceiver(TwoWire& wire, int sdaPin, int sclPin) : m_wire(wire)
{
    m_wire.onReceive(onReceive);
    m_wire.begin(m_address);
}


void BeatReceiver::onReceive(int bytes)
{
    pReceiver->receive(bytes);
}

void BeatReceiver::receive(int bytes)
{
    MSGEQ7Out event;
    static uint8_t buffer[255];


    m_wire.readBytes(buffer, min(bytes, static_cast<int>(sizeof(buffer))));
    if (bytes == event.bytes())
    {
        memcpy(&event, buffer, bytes);
        BeatReceiver& receiver = BeatReceiver::get();
        for_each(receiver.m_listeners.begin(), receiver.m_listeners.end(), [&](std::set<MSGEQ7Out::Listener*>::const_reference nextListener)
            { nextListener->notify(event); });
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

void BeatReceiver::addListener(MSGEQ7Out::Listener* pListener)
{
    m_listeners.insert(pListener);
}

void BeatReceiver::removeListener(MSGEQ7Out::Listener* pListener)
{
    m_listeners.erase(pListener);
}
