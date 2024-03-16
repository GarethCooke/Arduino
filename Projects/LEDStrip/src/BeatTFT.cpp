#include <Arduino.h>
#include <Adafruit_ST7735.h>
#include "BeatTFT.h"

BeatTFT::BeatTFT(uint8_t pin_cs, uint8_t pin_dc, uint8_t pin_rst, uint8_t pin_mosi, uint8_t pin_sclk)
    : m_pTFT(new Adafruit_ST7735(pin_cs, pin_dc, pin_mosi, pin_sclk, pin_rst))
{
    m_pTFT->initR(INITR_MINI160x80);

    m_pTFT->setRotation(1);
    m_pTFT->fillScreen(ST77XX_BLACK);

    // large block of text
    m_pTFT->fillScreen(ST77XX_BLACK);
    m_pTFT->setCursor(0, 0);
    m_pTFT->setTextColor(ST77XX_WHITE);
    m_pTFT->setTextWrap(true);
    m_pTFT->print("Test text.  Test text.  Test text.  Test text.  Test text.  Test text.  Test text.  Test text.  Test text.  Test text.  Test text.  Test text.");
}


BeatTFT::~BeatTFT()
{
    delete m_pTFT;
}


void BeatTFT::notify(const JsonDocument& settings)
{

}


void BeatTFT::notify(const MSGEQ7Out& evt)
{

}
