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

    for(int i = 0; i<config.stripLen; i++) {
        pixels.setPixelColor(i, pixels.Color(0,8,16));
    }
    
    zz = (zz+1)%10;
    for(int i = 0; i<config.stripLen; i++) {
      if(i % 10 == zz) {
        pixels.setPixelColor(i, pixels.Color(0,150,0));
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


