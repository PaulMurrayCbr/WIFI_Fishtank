#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

#include "base.hpp"
#include "config.hpp"
#include "webserver.hpp"

Runnable *Runnable::headRunnable = NULL;

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
