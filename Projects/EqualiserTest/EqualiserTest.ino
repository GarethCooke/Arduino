#include <Arduino.h>
#include <Adafruit_SPIDevice.h>
#include <Adafruit_I2CRegister.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_BusIO_Register.h>
#include <gfxfont.h>
#include <Adafruit_SPITFT_Macros.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_GrayOLED.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SoundEvent.h"
#include "BeatDisplay.h"

int strobePin = 5;    // Strobe Pin on the MSGEQ7
int resetPin = 4;    // Reset Pin on the MSGEQ7
int outPin = 6;   // Output Pin on the MSGEQ7

NetworkHost noHost;
BeatDisplay* pDisplay = NULL;

void setup() {
  Serial.begin(9600);
  Wire.begin(18, 9);
  pDisplay = new BeatDisplay(noHost);

  // Define our pin modes
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  pinMode(outPin, INPUT);

  // Create an initial state for our pins
  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, LOW);
  delay(1);

  // Reset the MSGEQ7 as per the datasheet timing diagram
  digitalWrite(resetPin, HIGH);
  delay(1);
  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);
  delay(1);
}

void loop() {
	SoundEvent::Initialiser nextVal;

    // Cycle through each frequency band by pulsing the strobe.
    for (int nSpectrum = 0; nSpectrum < SoundEvent::getBands(); nSpectrum++)
    {
        digitalWrite(strobePin, LOW);
        delayMicroseconds(100);                  // Delay necessary due to timing diagram
        long level = analogRead(outPin);
        nextVal = (level * 255) / 4095;
        digitalWrite(strobePin, HIGH);
        delayMicroseconds(1);                    // Delay necessary due to timing diagram  
    }


	pDisplay->notify(nextVal);
	//delay(100);
}
