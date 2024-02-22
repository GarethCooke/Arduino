#if defined(ESP8266) || defined(ESP32)

#include <Arduino.h>
#include <WiFiUdp.h>
#include "BeatUDPReceiver.h"


BeatUDPReceiver::BeatUDPReceiver()
{
}


void BeatUDPReceiver::read(int bytes)
{
    Serial.printf("Reading %d bytes\n", bytes);
    m_udp.read(m_buffer, min(bytes, BUF_SIZE));
}


void BeatUDPReceiver::flush()
{
    Serial.println("Flushing...");
    m_udp.flush();
}


void BeatUDPReceiver::handle()
{
    Serial.println("BeatUDPReceiver::handle()");
    int bytes = m_udp.parsePacket();
    Serial.printf("Found %d bytes\n", bytes);
    if (bytes)
        receive(bytes);
}
#endif
