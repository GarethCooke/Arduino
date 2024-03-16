#include "SettingsListener.h"
#include "MSGEQ7Out.h"

class Adafruit_ST7735;

class BeatTFT : public SettingsListener, public MSGEQ7Out::Listener
{
public:
    BeatTFT(uint8_t pin_cs, uint8_t pin_dc, uint8_t pin_rst, uint8_t pin_mosi, uint8_t pin_sclk);
    virtual ~BeatTFT();

    virtual void notify(const JsonDocument& settings);
    virtual void notify(const MSGEQ7Out& evt);

private:
    Adafruit_ST7735* m_pTFT;
};