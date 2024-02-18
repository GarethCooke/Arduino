#include <Arduino.h>
#include <memory>
#include <Wire.h>
#include "BeatReceive.h"

using std::for_each;


void BeatReceive::received(const MSGEQ7Out& evt)
{
    for_each(m_listeners.begin(), m_listeners.end(), [&](std::set<MSGEQ7Out::Listener*>::const_reference nextListener)
        { nextListener->notify(evt); });
}

void BeatReceive::addListener(MSGEQ7Out::Listener* pListener)
{
    m_listeners.insert(pListener);
}

void BeatReceive::removeListener(MSGEQ7Out::Listener* pListener)
{
    m_listeners.erase(pListener);
}
