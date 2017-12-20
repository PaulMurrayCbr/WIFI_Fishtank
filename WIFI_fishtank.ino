#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

#include "base.hpp"
#include "config.hpp"
#include "webserver.hpp"

void setup() {
#ifdef DEBUG
  Serial.begin(115200); // initializing Serial
  while (!Serial) ;
#endif
  LOG("\n\nInvoking setup");
  config.setup();

  LOGN(F("\nConnecting to "));
  LOG(config.ssid);

  WiFi.begin(config.ssid, config.password); // connecting to a WPA/WPA2 network

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    LOGN(F("."));
  }

  LOG(F("\nWiFi connected"));
  LOGN(F("SSID: "));
  LOG(WiFi.SSID());
  long rssi = WiFi.RSSI();
  LOGN(F("Signal Strength (RSSI): "));
  LOGN(rssi);
  LOG(F(" dBm"));
  LOGN(F("IP address: "));
  LOGN(WiFi.localIP());
  LOG();

  webserver.setup();
}

void loop() {
  webserver.loop();

}
