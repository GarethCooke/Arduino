#pragma once
#if defined(ESP8266) || defined(ESP32)


#include <BeatUDPComms.h>

class BeatUDPSender : public BeatUDPComms, public BeatSendImpl
{
public:
    BeatUDPSender() {}
    virtual void send(const MSGEQ7Out& evt);
};
#endif
