#pragma once

#include <memory>
#include <BeatWireComms.h>

#if defined(_UNIQUE_PTR_H)
#define std_ptr_type unique_ptr
#else
#define std_ptr_type auto_ptr
#endif

using std::std_ptr_type;

class BeatBroadcast : public MSGEQ7Out::Listener
{
public:
    BeatBroadcast(std_ptr_type<BeatSendImpl> pSender);
    virtual ~BeatBroadcast();
    virtual void notify(const MSGEQ7Out& evt);

private:
    BeatBroadcast(const BeatBroadcast&) = delete;
    BeatBroadcast& operator=(const BeatBroadcast&) = delete;

    std_ptr_type<BeatSendImpl> m_pSender;
};
