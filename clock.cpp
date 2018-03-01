#include <Arduino.h>

#include <Wire.h>
#include <RTClib.h>

RTC_DS1307 rtc;

#include "base.hpp"
#include "clock.hpp"

class ClockPvt {
    friend Clock;

    int timeOfDay_min = 0;
    long timeOfDay_sec = 0;

    void setup() {
      LOG("Initializing clock on SDA D4, SCL D3");
      Wire.begin(D4, D3);
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

    long getTimeOfDay_sec() {
      return timeOfDay_sec;
    }

    void update_tod() {
      DateTime t = getDateTime();
      timeOfDay_min = t.hour() * 60 + t.minute();
      timeOfDay_sec = t.hour() * 60L * 60L + t.minute() * 60L + t.second();
    }

    DateTime getDateTime() {
      return rtc.now();
    }

    void setDateTime(DateTime t) {
      rtc.adjust(t);
      update_tod();
      ClockListener::notifyAll(timeOfDay_min);
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

long Clock::getTimeOfDay_sec() {
  return clockPvt.getTimeOfDay_sec();
}

DateTime Clock::getDateTime() {
  return clockPvt.getDateTime();
}

void Clock::setDateTime(DateTime t) {
  return clockPvt.setDateTime(t);
}


