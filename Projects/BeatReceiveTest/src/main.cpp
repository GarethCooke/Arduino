#include <Arduino.h>
#include <Wire.h>
#include <MSGEQ7Out.h>
#include <BeatReceiver.h>


class SoundInterest : public MSGEQ7Out::Listener
{
public:
  virtual void notify(const MSGEQ7Out& evt)
  {
    evt.iterate_bands([](const char* frequency, unsigned int value, bool beat)
      {
        Serial.print(frequency);
        Serial.print("\t");
        Serial.print(value);
        Serial.print("\t");
        Serial.println(beat);
      }
    );
  };
};

SoundInterest soundInterest;

void setup()
{
  Serial.begin(9600);
  BeatReceiver::create(Wire, A4, A5);
  BeatReceiver::get().addListener(&soundInterest);
  Serial.println("Ready");
}

void loop()
{
}
