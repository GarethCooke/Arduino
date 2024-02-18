#pragma once

#include <BeatComms.h>

class TwoWire;

class BeatWireComms
{
public:
    BeatWireComms(TwoWire& wire);

protected:
    static constexpr const int m_address = 0x30;
    TwoWire& m_wire;
};
