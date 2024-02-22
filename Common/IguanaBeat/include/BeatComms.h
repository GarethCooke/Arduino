#pragma once

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
    void receive(int bytes);

protected:
    static constexpr const int BUF_SIZE = 255;
    BeatReceiver* m_pReceiver = NULL;
    uint8_t m_buffer[BUF_SIZE];

    virtual void read(int bytes) = 0;
    virtual void flush() = 0;
};