#if defined(ESP8266) || defined(ESP32)

#include <Arduino.h>
#include <WiFiUdp.h>
#include "BeatUDPSender.h"

void BeatUDPSender::send(const MSGEQ7Out& evt)
{
    Serial.println("Broadcasting event via UDP");
    Serial.printf("UDP begin: %d\n", m_udp.beginPacket("192.168.1.255", 2390));
    Serial.printf("UDP write: %d\n", m_udp.write(reinterpret_cast<const uint8_t*>(&evt), sizeof(evt)));
    Serial.printf("UDP end: %d\n", m_udp.endPacket());
}
#endif
