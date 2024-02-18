#pragma once

#include <BeatWireComms.h>

class BeatWireSender : public BeatWireComms, public BeatSendImpl
{
public:
    BeatWireSender(TwoWire& wire) : BeatWireComms(wire) {}
    virtual void send(const MSGEQ7Out& evt);
};
