#include "BeatDisplay.h"

class Adafruit_ST7735;

class BeatTFT : public BeatDisplay
{
public:
    BeatTFT(NetworkHost& host, uint8_t pin_cs, uint8_t pin_dc, uint8_t pin_rst, uint8_t pin_mosi, uint8_t pin_sclk);
    virtual ~BeatTFT();

protected:
    virtual Adafruit_GFX& display();
    virtual void resetDisplay();
    virtual void show();
    virtual int16_t height() const;
    virtual uint16_t getTextColour() const;
    virtual uint16_t getBarColour() const;

private:
    Adafruit_ST7735* m_pTFT;
};
