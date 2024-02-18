#include <Arduino.h>
#include "BeatBroadcast.h"

BeatBroadcast::BeatBroadcast(std_ptr_type<BeatSendImpl> pSender) : m_pSender(pSender.release())
{
}

BeatBroadcast::~BeatBroadcast()
{
}

void BeatBroadcast::notify(const MSGEQ7Out& evt)
{
    m_pSender->send(evt);
}
