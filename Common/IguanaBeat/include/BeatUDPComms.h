#pragma once

#include <BeatComms.h>

class BeatUDPComms
{
protected:
    BeatUDPComms() {}
    WiFiUDP m_udp;
};
