#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "base.hpp"
#include "lights.hpp"

#include "config.hpp"

#include "clock.hpp"

Lights lights;


class LightsPvt : ConfigListener, ClockListener  {
    friend Lights;
    const int MAX_STRIP = 120;

    Adafruit_NeoPixel pixels = Adafruit_NeoPixel(MAX_STRIP, D8, NEO_GRB + NEO_KHZ800);

    void setup() {
//      pinMode(D8, OUTPUT);
      LOG("pixel strip on D8");
      pixels.begin();
      pixels.clear();
      pixels.show();
    }

    void loop() {}

    void configChanged() {
      pixels.setBrightness(config.brightness);
      show();
    }

    void clockTick() {
      show();
    }

    int zz = 0;

    void show() {
      pixels.clear();

      long t1 = config.moonriseMins * 60L;
      long t2 = config.moonsetMins * 60L;
      long t = clock.getTimeOfDay_sec();

      if (t2 < t1) t2 += 24L * 60L * 60L;
      if (t < t1) t += 24L * 60L * 60L;
      
      float center = (float)(t - t1) / (float)(t2 - t1);
      center *= config.stripLen;

      for (int i = 0; i < config.stripLen; i++) {
        float d = i - center;
        d /= config.moonWidth / 2;

        float b = 1 - fabs(d); // brightest in the center

        if (b > 0) {
          b *= b; // gamma correction, kinda-sorta

          pixels.setPixelColor(i, pixels.Color(
                                 (int)(config.rgbR * b),
                                 (int)(config.rgbG * b),
                                 (int)(config.rgbB * b)));
        }
      }

      pixels.show();
    }
};

LightsPvt lightsPvt;


void Lights::setup() {
  lightsPvt.setup();
}

void Lights::loop() {
  lightsPvt.loop();
}


