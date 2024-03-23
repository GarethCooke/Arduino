#include "BeatSSD1306.h"

#define SCREEN_WIDTH 128	// OLED display width, in pixels
#define SCREEN_HEIGHT 32	// OLED display height, in pixels
#define OLED_RESET -1		// Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32


BeatSSD1306::BeatSSD1306(NetworkHost& host, TwoWire& wire)
    : BeatDisplay(host), m_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET)
{
    if (!m_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
        Serial.println(F("SSD1306 allocation failed"));
}


void BeatSSD1306::resetDisplay()
{
    m_display.clearDisplay();
}


void BeatSSD1306::show()
{
    m_display.display();
}


int16_t BeatSSD1306::height() const
{
    return m_display.height();
}


uint16_t BeatSSD1306::getTextColour() const
{
    return WHITE;
}


uint16_t BeatSSD1306::getBarColour() const
{
    return WHITE;
}
