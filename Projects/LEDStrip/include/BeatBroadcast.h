#pragma once

#include "SoundEvent.h"

class TwoWire;

class BeatBroadcast : public SoundEvent::Listener
{
public:
    BeatBroadcast(TwoWire &wire);
    virtual void notify(const SoundEvent::Output &evt);

private:
    static constexpr const int m_address = 0x30;
    TwoWire &m_wire;
};
