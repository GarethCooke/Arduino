#pragma once

#include <Adafruit_SSD1306.h>
#include "BeatDisplay.h"

class BeatSSD1306 : public BeatDisplay
{
public:
    BeatSSD1306(NetworkHost& host, TwoWire& wire);

protected:
    virtual Adafruit_GFX& display() { return m_display; };
    virtual void resetDisplay();
    virtual void show();
    virtual int16_t height() const;
    virtual uint16_t getTextColour() const;
    virtual uint16_t getBarColour() const;

private:
    Adafruit_SSD1306 m_display;
};
