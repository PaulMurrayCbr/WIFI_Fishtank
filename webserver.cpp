#include <Arduino.h>

#include <ESP8266WiFi.h>

#include "base.hpp"
#include "webserver.hpp"
#include "config.hpp"

WiFiServer server(80);

Webserver webserver;

extern byte favicon_ico[];
extern unsigned int favicon_ico_len;

void Webserver::setup() {
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

  server.begin();
  LOG("server started");
}


const int MAXLN = 256;
char ln[MAXLN + 1];
int contentLength;
char contentType[MAXLN + 1];
char requestMethod[MAXLN + 1];
char requestURL[MAXLN + 1];
char requestVersion[MAXLN + 1];


boolean readLine(WiFiClient &client) {
  int i = 0;
  boolean cr = false;

  for (;;) {
    if (!client.connected()) return false;
    if (!client.available()) continue;
    char c = client.read();
    switch (c) {
      case '\r':
        if (cr) {
          // should never happen
          if (i < MAXLN) ln[i++] = '\r';
        }
        cr  =  true;
        break;

      case '\n':
        if (cr) {
          // there's always room for the trailing NUL
          ln[i++]  =  '\0';
          return true;
        }
      // else fall through, treat \n like a regular character
      // should never happen

      default:
        if (cr) {
          // should never happen
          if (i < MAXLN) ln[i++] = '\r';
          cr  =  false;
        }
        if (i < MAXLN) ln[i++] = c;
        break;
    }
  }
}

void parseRequestLine() {
  char *p = ln;
  char *q;

  q = requestMethod;
  while (*p && (q - requestMethod) < MAXLN && *p != ' ') {
    *q++ = *p++;
  }
  *q = '\0';
  if (*p) p++;

  q = requestURL;
  while (*p && (q - requestURL) < MAXLN && *p != ' ') {
    *q++ = *p++;
  }
  *q = '\0';
  if (*p) p++;

  q = requestVersion;
  while (*p && (q - requestVersion) < MAXLN && *p != ' ') {
    *q++ = *p++;
  }
  *q = '\0';
  if (*p) p++;

  LOGN(F("request method: "));
  LOG(requestMethod);

  LOGN(F("request URL: "));
  LOG(requestURL);

  LOGN(F("request Version: "));
  LOG(requestVersion);

}

boolean readRequestHeader(WiFiClient &client) {
  if (!readLine(client)) return false; // read the http request line

  LOGN(F("Request line: "));
  LOG(ln);
  parseRequestLine();

  contentLength  =  -1;  // initialize
  contentType[0] = '\0';

  if (strcmp(requestVersion, "HTTP/1.1")) return false;

  // we are assuming http 1.1. Read lines until we get a blank line
  do {
    if (!readLine(client)) return false; // read a header
    LOG(ln);
    if (strncmp(ln, "Content-Length: ", 16) == 0) {
      contentLength  =  atoi(ln + 16);
    }
    if (strncmp(ln, "Content-Type: ", 14) == 0) {
      strncpy(contentType, ln + 14, MAXLN);
    }
  }
  while (strlen(ln) > 0);
  return true;
}


const char unsupportedMethod[] PROGMEM = R"=====(HTTP/1.1 405 Method Not Allowed
Content-Type: text/html
Connection: close

<html><body>This service only accepts GET requests</body></html>
)=====";

const char staticOK[] PROGMEM = R"=====(HTTP/1.1 200 OK
Content-Type: text/html
Connection: close

<html><body>Yes, we have HTML.</body></html>
)=====";


void reply(WiFiClient &client, const char* p) {
    char ch;
    while(ch =  pgm_read_byte_near(p++))  client.print(ch);
}

void Webserver::loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  LOG("new client");
  while (!client.available()) {
    delay(1);
  }

  if (!readRequestHeader(client)) {
    client.stop();
    return;
  }
  else if (strcmp(requestMethod, "GET")) {
    reply(client, unsupportedMethod);
    return;
  }
  else {
    reply(client, staticOK);
  }
  
  // give the web browser time to receive the data
  delay(1);
  // close the connection:
  client.stop();
}

void Webserver::configChanged() {
  
}


