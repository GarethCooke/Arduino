#include <MSGEQ7Out.h>

class BeatSendImpl
{
public:
    virtual void send(const MSGEQ7Out& evt) = 0;
};

class BeatReceiveImpl
{
public:
    class BeatReceiver
    {
    public:
        virtual void received(const MSGEQ7Out& evt) = 0;
    };

    void registerReceiver(BeatReceiver& receiver) { m_pReceiver = &receiver; }

protected:
    BeatReceiver* m_pReceiver = NULL;
};