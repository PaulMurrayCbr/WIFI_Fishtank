#include <Arduino.h>
#include <EEPROM.h>

#include "base.hpp"
#include "usbtalk.hpp"

UsbTalk usbTalk;

void UsbTalk::setup() {
#ifndef DEBUG
  // We can't wait for serial, because there may be no USB connected
  Serial.begin(115200); // initializing Serial
  delay(500);
#endif

  if (Serial) {
    Serial.println(F("Ready to configure"));
    Serial.println(F("S - enter SSID"));
    Serial.println(F("P - enter Password"));
  }
}


void UsbTalk::loop() {
  if (!Serial || !Serial.available()) return;
}

