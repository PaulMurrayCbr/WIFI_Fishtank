#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "base.hpp"
#include "lights.hpp"

#include "config.hpp"

#include "clock.hpp"

Lights lights;


class LightsPvt : ConfigListener, ClockListener  {
  friend Lights;

  const int MAX_STRIP = 60;

  Adafruit_NeoPixel pixels = Adafruit_NeoPixel(MAX_STRIP, D2, NEO_GRB + NEO_KHZ800);

  void setup() {
    pinMode(D2, OUTPUT);
    LOG("pixel strip on D2");
    pixels.begin();
    pixels.clear();
    pixels.show();
  }

  void loop() {}

  void configChanged() {
    show();  
  }

  void clockTime(int timeOfDay_min) {
    show();
  }

  int zz = 0;

  void show() {
    pixels.clear();

    int t1 = config.moonriseMins;
    int t2 = config.moonsetMins; 

    if(t2 < t1) t2 += 24*60;
    int t = clock.getTimeOfDay_min();

    if(t < t1 || t > t2) {
      pixels.show();
      return;
    }

    float center = (float)(t - t1) / (float)(t2-t1);
    center = center * (config.stripLen + config.moonWidth) - config.moonWidth/2.0;

    for(int i = 0; i<config.stripLen; i++) {
      float d = i - center;
      d /= config.moonWidth;
      if(d >= -.5 && d <= .5) {
        float b = (d+.5) * (.5-d); // a parabola
        b *= 4;
        b = sqrt(b);

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


