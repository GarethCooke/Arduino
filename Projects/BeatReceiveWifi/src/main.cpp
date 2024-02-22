#include <Arduino.h>
#include <ESPAsyncWebSrv.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_SPIDevice.h>
#include <MSGEQ7Out.h>
#include <BeatReceive.h>
#include <BeatDisplay.h>
#include <BeatUDPReceiver.h>


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


class DelaySoundInterest : public MSGEQ7Out::Listener
{
public:
	virtual void notify(const MSGEQ7Out& evt)
	{
		delay(1000);
	};
};

SoundInterest soundInterest;
NoOpSoundInterest noOpSoundInterest;
DelaySoundInterest delaySoundInterest;
NetworkHost noNetwork;
BeatReceive beatReceiver;
BeatUDPReceiver udpReceiver;

// BeatDisplay display(noNetwork, Wire);

void setup()
{
	Serial.begin(115200);
	udpReceiver.registerReceiver(beatReceiver);
	beatReceiver.addListener(&soundInterest);
	// beatReceiver.addListener(&noOpSoundInterest);
	// beatReceiver.addListener(&display);
	beatReceiver.addListener(&delaySoundInterest);
	Serial.println("Ready");
}

void loop()
{
	udpReceiver.handle();
}
