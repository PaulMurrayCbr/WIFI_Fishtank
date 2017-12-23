#include <Arduino.h>

#include <ESP8266WiFi.h>

#include "base.hpp"
#include "webserver.hpp"
#include "config.hpp"

WiFiServer server(80);

Webserver webserver;

extern byte favicon_ico[];
extern unsigned int favicon_ico_len;

const char unsupportedMethod[] PROGMEM = R"=====(HTTP/1.1 405 Method Not Allowed
Content-Type: text/html
Connection: close

<html><body>This service only accepts GET requests</body></html>
)=====";

const char notFound[] PROGMEM = R"=====(HTTP/1.1 404 Not Found
Content-Type: text/html
Connection: close

<html><body>Not found.</body></html>
)=====";

const char homePage1[] PROGMEM = R"=====(HTTP/1.1 200 OK
Content-Type: text/html
Connection: close

<!DOCTYPE html 
      PUBLIC "-//W3C//DTD HTML 4.01//EN"
      "http://www.w3.org/TR/html4/strict.dtd">
<html lang="en-US">
<head profile="http://www.w3.org/2005/10/profile">
<link rel="icon" 
      type="image/x-icon" 
      href="/favicon.ico" />
<title>Fishtank Moon</title>
</head> 
<body>
<h1>Fishtank Moon</h1>
<p>
Wecome to fishtank moon.
</p>
)=====";

const char homePage2[] PROGMEM = R"=====(
</body>
</html>
)=====";

class WebserverPvt {
  friend Webserver;

  enum State {
    CONNECTING, CONNECTED
  }
  state = CONNECTING;

  static const int MAXLN = 256;
  char ln[MAXLN + 1];
  int contentLength;
  char contentType[MAXLN + 1];
  char requestMethod[MAXLN + 1];
  char requestURL[MAXLN + 1];
  char requestVersion[MAXLN + 1];

  void setup();
  void mainPage(WiFiClient &client);
  void favicon(WiFiClient &client);
  
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

#ifdef DEBUG
    LOGN("Request Method: ");
    LOG(requestMethod);
    LOGN("Request URL: ");
    LOG(requestURL);
    LOGN("Request Version: ");
    LOG(requestVersion);

#endif    
  }
  
  boolean readRequestHeader(WiFiClient &client) {
    if (!readLine(client)) return false; // read the http request line
    parseRequestLine();
  
    contentLength  =  -1;  // initialize
    contentType[0] = '\0';
  
    if (strcmp(requestVersion, "HTTP/1.1")) return false;
  
    // we are assuming http 1.1. Read lines until we get a blank line
    do {
      if (!readLine(client)) return false; // read a header
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

  void reply(WiFiClient &client, const char* p) {
    LOG("REPLY [");
    char ch; 
    while(ch =  pgm_read_byte_near(p++)) {  
      client.print(ch); 
      LOGN(ch);
    }
    LOG("\n]");
  }

  void reply(WiFiClient &client, const __FlashStringHelper* p) {
    reply(client, (const char *) p);
  }
  
  void doConnecting();

  void doConnected() {
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

    if(strcmp(requestURL, "/") == 0) {
      mainPage(client);
    }
    else if(strcmp(requestURL, "/favicon.ico") == 0) {
      favicon(client);
    }
    else {
      reply(client, notFound);
    }
    
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }

  void loop() {
    switch(state) {
      case CONNECTING: doConnecting(); break;
      case CONNECTED: doConnected(); break;
      
    }
  }
  
  void configChanged() {
    
  }

} webserverPvt;

// this gear has to be declared extrernally, because the F macro just *will* not cooperate with gear.

void WebserverPvt::setup() {
  LOGN(F("\nConnecting to "));
  LOG(config.ssid);

  WiFi.begin(config.ssid, config.password); // connecting to a WPA/WPA2 network

  state = CONNECTING;
}

void WebserverPvt::doConnecting() {
  static uint32_t ms = 0;
  
  if (WiFi.status() != WL_CONNECTED) {
    if(millis() - ms >= 2000) {
      LOGN(F("Attempting to connect to "));
      LOG(WiFi.SSID());
    }
    ms = millis();
    return;
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
  state = CONNECTED;
}


void Webserver::setup() {
  webserverPvt.setup();
}

void Webserver::loop() {
  webserverPvt.loop();
}

void Webserver::configChanged() {
  webserverPvt.configChanged();  
}

void WebserverPvt::mainPage(WiFiClient &client) {
  log(sizeof(homePage1));
  reply(client, homePage1);
  log(sizeof(homePage2));
  reply(client, homePage2);
}

void WebserverPvt::favicon(WiFiClient &client) {
  reply(client, F("HTTP/1.1 200 OK\r\n"));
  reply(client, F("Content-Type: img/x-icon\r\n"));
  reply(client, F("Content-Length: "));
  client.print(favicon_ico_len);
  reply(client, F("\r\n"));
  reply(client, F("Connection: close\r\n"));
  reply(client, F("\r\n"));

  for(int i = 0; i<favicon_ico_len; i++) {
    client.print((char)pgm_read_byte_near(favicon_ico+i));
  }
}

