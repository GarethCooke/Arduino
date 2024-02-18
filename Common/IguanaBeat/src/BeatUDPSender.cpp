#include <Arduino.h>
#include <WiFiUdp.h>
#include "BeatUDPSender.h"

WiFiUDP Udp;

void BeatUDPSender::send(const MSGEQ7Out& evt)
{
    Serial.println("Broadcasting event via UDP");
    Serial.printf("UDP begin: %d\n", Udp.beginPacket("192.168.1.255", 2390));
    Serial.printf("UDP write: %d\n", Udp.write(reinterpret_cast<const uint8_t*>(&evt), sizeof(evt)));
    Serial.printf("UDP end: %d\n", Udp.endPacket());
}
