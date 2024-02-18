#pragma once

#include <MSGEQ7Out.h>

class TwoWire;

class BeatBroadcast : public MSGEQ7Out::Listener
{
public:
    BeatBroadcast(TwoWire &wire);
    virtual ~BeatBroadcast();
    virtual void notify(const MSGEQ7Out &evt);

    static constexpr const int address() { return m_address; }

private:
    static constexpr const int m_address = 0x30;
    TwoWire &m_wire;
};
