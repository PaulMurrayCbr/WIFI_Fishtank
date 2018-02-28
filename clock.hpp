#ifndef WFFT_CLOCK
#define WFFT_CLOCK 1

#include "RTClib.h"

class Clock: public Runnable {
  public:
    void setup();
    void loop();

    int getTimeOfDay_min();
    DateTime getDateTime();
};

class ClockListener {
  static ClockListener *headClockListener;
  ClockListener *nextClockListener;

  virtual void clockTime(int timeOfDay_min) = 0;

  public:
  ClockListener() {
    nextClockListener = headClockListener;
    headClockListener = this;
  }
  
  static void notifyAll(int timeOfDay_min) {
    for (ClockListener *r = headClockListener; r; r = r->nextClockListener)
      r->clockTime(timeOfDay_min);
  }

};

extern Clock clock;

#endif
