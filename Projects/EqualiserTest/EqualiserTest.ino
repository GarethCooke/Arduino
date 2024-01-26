#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
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
#include "Beatbox.h"

int strobe_pin = 2;    // Strobe Pin on the MSGEQ7
int reset_pin = 5;    // Reset Pin on the MSGEQ7
int out_pin = 1;   // Output Pin on the MSGEQ7

NetworkHost noHost;
BeatDisplay* pDisplay = NULL;

struct simple_msgeq7
{
    static void create()
    {
        // Define our pin modes
        pinMode(strobe_pin, OUTPUT);
        pinMode(reset_pin, OUTPUT);
        pinMode(out_pin, INPUT);

        // Create an initial state for our pins
        digitalWrite(reset_pin, LOW);
        digitalWrite(strobe_pin, LOW);
        delay(1);

        // Reset the MSGEQ7 as per the datasheet timing diagram
        digitalWrite(reset_pin, HIGH);
        delay(1);
        digitalWrite(reset_pin, LOW);
        digitalWrite(strobe_pin, HIGH);
        delay(1);
    }

    static void handle()
    {
        SoundEvent::Initialiser nextVal;

        // Cycle through each frequency band by pulsing the strobe.
        for (int nSpectrum = 0; nSpectrum < SoundEvent::getBands(); nSpectrum++)
        {
            digitalWrite(strobe_pin, LOW);
            delayMicroseconds(100);                  // Delay necessary due to timing diagram
            long level = analogRead(out_pin);
            nextVal = (level * 255) / 4095;
            digitalWrite(strobe_pin, HIGH);
            delayMicroseconds(1);                    // Delay necessary due to timing diagram  
        }


        pDisplay->notify(nextVal);
        //delay(100);
    }
};

struct beater
{
    static void create()
    {
        Beatbox::create(reset_pin, strobe_pin, out_pin);
        Beatbox::get().addListener(pDisplay);
        Beatbox::get().start();
    }
    static void handle() {}
};

//typedef simple_msgeq7 msgeq7_handler;
typedef beater msgeq7_handler;

void setup()
{
    Serial.begin(9600);
    Wire.begin(18, 9);
    pDisplay = new BeatDisplay(noHost);

    msgeq7_handler::create();
}

void loop()
{
    msgeq7_handler::handle();
}
