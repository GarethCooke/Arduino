#pragma once
#if defined(ESP8266) || defined(ESP32)


#include <BeatUDPComms.h>

class BeatUDPReceiver : public BeatUDPComms, public BeatReceiveImpl
{
public:
    BeatUDPReceiver();
    void handle();

protected:
    virtual void read(int bytes);
    virtual void flush();
};
#endif
