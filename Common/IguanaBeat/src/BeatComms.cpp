#include <Arduino.h>
#include <BeatComms.h>


void BeatReceiveImpl::receive(int bytes)
{
    MSGEQ7Out event;

    if (bytes == event.bytes())
    {
        read(bytes);
        memcpy(&event, m_buffer, bytes);
        if (m_pReceiver)
            m_pReceiver->received(event);
    }
    else
    {
        // dont recognise this message - discard bytes to allow other message processing
        flush();
    }
}
