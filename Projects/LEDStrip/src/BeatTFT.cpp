#include <Arduino.h>
#include <Adafruit_ST7735.h>
#include "BeatTFT.h"

BeatTFT::BeatTFT(NetworkHost& host, uint8_t pin_cs, uint8_t pin_dc, uint8_t pin_rst, uint8_t pin_mosi, uint8_t pin_sclk)
    : BeatDisplay(host), m_pTFT(new Adafruit_ST7735(pin_cs, pin_dc, pin_mosi, pin_sclk, pin_rst))
{
    m_pTFT->initR(INITR_BLACKTAB);
}


BeatTFT::~BeatTFT()
{
    delete m_pTFT;
}


Adafruit_GFX& BeatTFT::display()
{
    return *m_pTFT;
}


void BeatTFT::resetDisplay()
{
    m_pTFT->fillScreen(ST77XX_BLACK);
}


void BeatTFT::show()
{
}


int16_t BeatTFT::height() const
{
    return m_pTFT->height();
}


uint16_t BeatTFT::getTextColour() const
{
    return ST77XX_YELLOW;
}


uint16_t BeatTFT::getBarColour() const
{
    return ST77XX_GREEN;
}