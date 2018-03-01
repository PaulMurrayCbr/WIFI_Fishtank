#ifndef WFFT_CLOCK
#define WFFT_CLOCK 1

#include "RTClib.h"

class Clock: public Runnable {
  public:
    void setup();
    void loop();

    int getTimeOfDay_min();
    long getTimeOfDay_sec();
    DateTime getDateTime();
    void setDateTime(DateTime t);
};

class ClockListener {
  static ClockListener *headClockListener;
  ClockListener *nextClockListener;

  protected:
  virtual void clockTick() = 0;

  public:
  ClockListener() {
    nextClockListener = headClockListener;
    headClockListener = this;
  }
  
  static void notifyAll(int timeOfDay_min) {
    for (ClockListener *r = headClockListener; r; r = r->nextClockListener)
      r->clockTick();
  }

};

extern Clock clock;

#endif
