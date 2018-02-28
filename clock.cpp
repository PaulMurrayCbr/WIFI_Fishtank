#include <Arduino.h>

#include <Wire.h>
#include <RTClib.h>

RTC_DS1307 rtc;

#include "base.hpp"
#include "clock.hpp"

class ClockPvt {
    friend Clock;

    int timeOfDay_min = 0;

    void setup() {
      LOG("Initializing clock on SDA D3, SCL D4");
      Wire.begin(D3, D4);
    }

    void loop() {
      static uint32_t ms = 0;

      // update every five seconds
      if (millis() - ms < 5000) return;
      ms = millis();

      update_tod();

      ClockListener::notifyAll(timeOfDay_min);
    }

    int getTimeOfDay_min() {
      return timeOfDay_min;
    }

    void update_tod() {
      DateTime t = getDateTime();
      timeOfDay_min = t.hour() * 24 + t.minute();
    }

    DateTime getDateTime() {
      return rtc.now();
    }

} clockPvt;


Clock clock;
ClockListener *ClockListener::headClockListener = NULL;

void Clock::setup() {
  clockPvt.setup();
}

void Clock::loop() {
  clockPvt.loop();
}

int Clock::getTimeOfDay_min() {
  return clockPvt.getTimeOfDay_min();
}

DateTime Clock::getDateTime() {
  return clockPvt.getDateTime();
}


