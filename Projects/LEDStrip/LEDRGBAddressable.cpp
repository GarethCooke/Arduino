#include <WS2812FX.h>
#include "LEDRGBAddressable.h"


LEDRGBAddressable::LEDRGBAddressable(uint8_t d_pin)
{
    int led_count = 30;

    // Parameter 3 = pixel type flags, add together as needed:
    //   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    //   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    //   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
    //   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
    //   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
    m_pWS2812fx = new WS2812FX(led_count, d_pin, NEO_GRB + NEO_KHZ800);

    m_pWS2812fx->init();
    m_pWS2812fx->setBrightness(255);
    //m_pWS2812fx->setMode(FX_MODE_RAINBOW_CYCLE);
    //m_pWS2812fx->setMode(FX_MODE_THEATER_CHASE);
    m_pWS2812fx->setMode(FX_MODE_THEATER_CHASE_RAINBOW);
}

LEDRGBAddressable::~LEDRGBAddressable()
{
    delete m_pWS2812fx;
}


void LEDRGBAddressable::notify(const JsonDocument& settings)
{
}


void LEDRGBAddressable::notify(const Beatbox::Event& evt)
{
    if (evt.beatDetected())
        m_pWS2812fx->trigger();
}


void LEDRGBAddressable::handle()
{
    m_pWS2812fx->service();
}
