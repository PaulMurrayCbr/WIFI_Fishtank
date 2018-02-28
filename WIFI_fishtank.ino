#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

#include "base.hpp"
#include "config.hpp"
#include "webserver.hpp"

#include "clock.hpp"

Runnable *Runnable::headRunnable = NULL;

class Foo: public ClockListener {
  void clockTime(int timeOfDay_min) {
    LOGN("clocktime ");
    LOGN(timeOfDay_min);
    LOGN('=');
    LOGN(timeOfDay_min/60);
    LOGN(':');
    LOG(timeOfDay_min%60);
  }
} foo;

class Bar: public ConfigListener {
  void configChanged() {
    LOG("Config updated");
  }
} bar;


void setup() {
#ifdef DEBUG
  Serial.begin(115200); // initializing Serial
  while (!Serial) ;
#endif

  LOG("\n\nReading config");
  config.setup();

  Runnable::setupAll();
}

void loop() {
  Runnable::loopAll();
}
