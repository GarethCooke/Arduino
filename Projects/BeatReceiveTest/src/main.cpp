#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_SPIDevice.h>
#include <MSGEQ7Out.h>
#include <BeatReceive.h>
#include <BeatDisplay.h>
#include <BeatWireReceiver.h>


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
    if (evt.beatDetected())
      Serial.println("BEAT!!!");
  };
};


class NoOpSoundInterest : public MSGEQ7Out::Listener
{
public:
  virtual void notify(const MSGEQ7Out& evt)
  {
  };
};

SoundInterest soundInterest;
NoOpSoundInterest noOpSoundInterest;
NetworkHost noNetwork;
BeatReceive beatReceiver;

// BeatDisplay display(noNetwork, Wire);

void setup()
{
  Serial.begin(9600);
  BeatWireReceiver::create(Wire, A4, A5);
  BeatWireReceiver::get().registerReceiver(beatReceiver);
  beatReceiver.addListener(&soundInterest);
  // beatReceiver.addListener(&noOpSoundInterest);
  // beatReceiver.addListener(&display);
  Serial.println("Ready");
}

void loop()
{
}
