#pragma once

#include <BeatWireComms.h>

class BeatWireSender : public BeatWireComms, public BeatSendImpl
{
public:
    BeatWireSender(TwoWire& wire);
    virtual void send(const MSGEQ7Out& evt);

private:
#if defined(ESP32)
    QueueHandle_t m_queue;
    static void senderTask(void* pvParameters);
#endif

    BeatWireSender(const BeatWireSender&) = delete;
    BeatWireSender& operator=(const BeatWireSender&) = delete;

    void sender(const MSGEQ7Out& evt);
};
