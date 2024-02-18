#pragma once

#include <set>
#include "BeatBroadcast.h"

using std::set;

class BeatReceive : public BeatReceiveImpl::BeatReceiver
{
public:
    void addListener(MSGEQ7Out::Listener* pListener);
    void removeListener(MSGEQ7Out::Listener* pListener);

    virtual void received(const MSGEQ7Out& evt);

private:
    std::set<MSGEQ7Out::Listener*> m_listeners;
};
