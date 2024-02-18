#pragma once

#include <BeatUDPComms.h>

class BeatUDPSender : public BeatUDPComms, public BeatSendImpl
{
public:
    BeatUDPSender() {}
    virtual void send(const MSGEQ7Out& evt);
};
