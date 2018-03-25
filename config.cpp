#include <Arduino.h>
#include <EEPROM.h>

#include "base.hpp"
#include "config.hpp"

#include "DEFAULT_SSID.hpp"

Config config;

ConfigListener *ConfigListener::headConfigListener = NULL;



void Config::setup() {
  LOG(F("Reading config from EEPROM"));
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
    strncpy(config.ssid, DEFAULT_SSID, sizeof(config.ssid));
    strncpy(config.password, DEFAULT_PASSWORD, sizeof(config.password));

    config.moonriseMins = 18*60;
    config.moonsetMins = 6*60;
    config.stripLen = 50;
    config.moonWidth = 5;

    config.brightness = 255;
    config.rgbR = 192;
    config.rgbG = 192;
    config.rgbB = 255;

    save();
  }
}

void Config::save() {
  LOG(F("Saving config to EEPROM"));
  ConfigListener::notifyAll();
}

