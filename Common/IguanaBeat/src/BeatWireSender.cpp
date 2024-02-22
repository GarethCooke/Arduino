#include <Wire.h>
#include "BeatWireSender.h"


void BeatWireSender::sender(const MSGEQ7Out& evt)
{
    m_wire.beginTransmission(m_address);
    m_wire.write(evt.toBytes(), evt.bytes());
    m_wire.endTransmission();
}


#if defined(ESP8266) || defined(ESP32)
BeatWireSender::BeatWireSender(TwoWire& wire) : BeatWireComms(wire), m_queue(xQueueCreate(2, sizeof(MSGEQ7Out)))
{
    static constexpr auto STACK_SIZE = 2000;
    static TaskHandle_t xHandle = NULL;

    xTaskCreate(senderTask, "BeaWire sender", STACK_SIZE, this, 1, &xHandle);
}


void BeatWireSender::send(const MSGEQ7Out& evt)
{
    xQueueSendToBack(m_queue, &evt, 0);

}

void BeatWireSender::senderTask(void* pvParameters)
{
    delay(1000);
    BeatWireSender* pThis = reinterpret_cast<BeatWireSender*>(pvParameters);
    MSGEQ7Out evt;
    while (true)
    {
        yield();
        while (xQueueReceive(pThis->m_queue, &evt, 0) == pdTRUE)
            pThis->sender(evt);
    }
}
#else
BeatWireSender::BeatWireSender(TwoWire& wire) : BeatWireComms(wire)
{

}


void BeatWireSender::send(const MSGEQ7Out& evt)
{
    sender(evt);
}
#endif
