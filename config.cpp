#include <Arduino.h>
#include <EEPROM.h>

#include "base.hpp"
#include "config.hpp"

Config config;

void Config::setup() {
  LOG(F("Reading congfig from EEPROM"));
  EEPROM.begin(0);
  for (int i = 0; i < sizeof(config); i++) {
    ((byte *)&config)[i] = EEPROM.read(i);
  }

  if (config.magic_number == config.MAGIC_NUMBER) {
    LOG(F("Configuration number matches ok"));
  }
  else {
    LOG(F("Initializing config"));
    config.magic_number = config.MAGIC_NUMBER;
    strncpy(config.ssid, "Unit 61", sizeof(config.ssid));
    strncpy(config.password, "Bing.zal.mir3", sizeof(config.ssid));

    save();
  }
}

void Config::save() {
  LOG(F("Saving config to EEPROM"));
}

