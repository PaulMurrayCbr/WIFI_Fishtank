#include <Arduino.h>

#include <stdlib.h>

#include <ESP8266WiFi.h>

#include "base.hpp"
#include "webserver.hpp"
#include "config.hpp"

#include "clock.hpp"

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
<link rel="icon" href="/favicon.ico" />
<link rel="shortcut icon" href="/favicon.ico" />
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
  static const int MAXPARAMS = 6;
  
  char ln[MAXLN + 1];
  int contentLength;
  char contentType[MAXLN + 1];
  char requestMethod[MAXLN + 1];
  char requestURL[MAXLN + 1];
  char requestVersion[MAXLN + 1];

  char parsed[MAXLN+1];

  char *requestPage;
  int requestParams;
  char *paramName[MAXPARAMS+1];
  char *paramValue[MAXPARAMS+1];


  void setup();
  void mainPage(WiFiClient &client);
  void setTime(WiFiClient &client);
  void setSchedule(WiFiClient &client);
  void setMoon(WiFiClient &client);
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

  void wwwdecode(char *s) {
    char *out = s;
    while(*s) {
      if(*s == '%') {
        char foo = s[3];
        s[3] = '\0';
        *out++ = (char) strtol(s+1, NULL, 16);
        s += 3;
        *s = foo;
      }
      else {
        *out++ = *s++;
      }
    }
    *out = '\0';
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
    strcpy(parsed, requestURL);
    p = parsed;

    requestPage = p;
    char delim;
    while((delim = *p) && delim != '?') p++;
    *p = 0;

    LOGN("page is [");
    LOGN(requestPage);
    LOGN("] ");
    LOG(delim);
    
    for(requestParams = 0; (delim=='?'||delim=='&') && requestParams < MAXPARAMS; requestParams++) {
      paramName[requestParams] = ++p;
      while((delim = *p) && delim != '&' && delim  != '=') p++;
      *p=0;
      wwwdecode(paramName[requestParams]);
      LOGN("param is [");
      LOGN(paramName[requestParams]);
      LOGN("] ");
      LOG(delim);
      if(delim == '=') {
        paramValue[requestParams] = ++p;
        while((delim = *p) && delim != '&') p++;
        *p = '\0';
        wwwdecode(paramValue[requestParams]);
        LOGN("param value is [");
        LOGN(paramValue[requestParams]);
        LOGN("] ");
        LOG(delim);
      }
      else {
        paramValue[requestParams] = p;
        LOG("no param value");
      }
    }

    paramName[requestParams] = NULL;
    paramValue[requestParams] = NULL;


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
//    LOG("REPLY [");
    char ch; 
    while(ch =  pgm_read_byte_near(p++)) {  
      client.print(ch); 
//      LOGN(ch);
    }
//    LOG("\n]");
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

    if(strcmp(requestPage, "/") == 0) {
      mainPage(client);
    }
    else if(strcmp(requestPage, "/favicon.ico") == 0) {
      favicon(client);
    }
    else if(strcmp(requestPage, "/setTime") == 0) {
      setTime(client);
    }
    else if(strcmp(requestPage, "/setSchedule") == 0) {
      setSchedule(client);
    }
    else if(strcmp(requestPage, "/setMoon") == 0) {
      setMoon(client);
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

  char *itoa2(int val, char *s) {
    s[0] = '0' + (val/10) % 10; 
    s[1] = '0' + (val) % 10; 
    s[2] = '\0';
    return s;
  }

  char *mins2s(int val, char *s) {
    itoa2(val/60, s);
    s[2] = ':';
    itoa2(val%60, s+3);
    return s;
  }

  int s2mins(char *s) {
    return
      (s[0]-'0') * 600 +
      (s[1]-'0') * 60 +
      (s[3]-'0') * 10 +
      (s[4]-'0') ;
  }

  int findParam(char *s) {
    for(int i = 0; i<requestParams; i++) {
      if(!strcmp(s, paramName[i])) return i;
    }
    return -1;
  }

  boolean hasParam(char *s) {
    int p = findParam(s);
    return findParam(s) != -1 && *paramValue[p];
  }

  char *getParam(char *s) {
    return paramValue[findParam(s)];
  }

  int getParamInt(char *s) {
    return atoi(getParam(s));
  }

  int getParamMins(char *s) {
    int ret = 0;
    char *p = getParam(s);
    ret += (int)(p[0]-'0') * 600;
    ret += (int)(p[1]-'0') * 60;
    ret += (int)(p[3]-'0') * 10;
    ret += (int)(p[4]-'0');
    return ret;
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
  reply(client, homePage1);

  DateTime dt = clock.getDateTime();
  char buf[10];

  reply(client, F("<h2>Set time</h2>"));
  
  client.print("<p>Current date/time: ");
  client.print(itoa2(dt.day(), buf)); 
  client.print('/');
  client.print(itoa2(dt.month(), buf)); 
  client.print('/');
  client.print((int)dt.year()); 
  client.print("&nbsp;");
  client.print(itoa2(dt.hour(), buf)); 
  client.print(':');
  client.print(itoa2(dt.minute(), buf)); 
  client.print(':');
  client.print(itoa2(dt.second(), buf)); 
  reply(client, F("</p>"));
  
  reply(client, F("<form  method='get' action='/setTime'>"));  
  reply(client, F("Date "));
  reply(client, F("<input name='date' type='date' value='"));
  client.print(itoa(dt.year(), buf, 10));
  client.print('-');
  client.print(itoa2(dt.month(), buf));
  client.print('-');
  client.print(itoa2(dt.day(), buf));
  reply(client, F("'></input>"));
  reply(client, F(" Time "));
  reply(client, F("<input name='time' type='time' value='"));
  client.print(itoa2(dt.hour(), buf));
  client.print(':');
  client.print(itoa2(dt.minute(), buf));
  reply(client, F("'></input>"));
  reply(client, F("<input name='set' value='Set Time' type='submit'></input>"));
  reply(client, F("</form>"));  

  reply(client, F("<h2>Set schedule</h2>"));
  reply(client, F("<p>Current schedule rise: "));
  client.print(mins2s(config.moonriseMins, buf));
  reply(client, F(", set: "));
  client.print(mins2s(config.moonsetMins, buf));
  reply(client, F("</p>"));
  
  reply(client, F("<form method='get' action='/setSchedule'>"));  
  reply(client, F("Moonrise "));
  reply(client, F("<input name='moonrise' type='time' value='"));
  client.print(mins2s(config.moonriseMins, buf));
  reply(client, F("'></input>"));
  reply(client, F("Moonset "));
  reply(client, F("<input name='moonset' type='time' value='"));
  client.print( mins2s(config.moonsetMins, buf));
  reply(client, F("'></input>"));
  reply(client, F("<input name='set' value='Set Schedule' type='submit'></input>"));
  reply(client, F("</form>"));  

  reply(client, F("<h2>Moon</h2>"));
  // brightness, RGB, width
  reply(client, F("<p>Current length: "));
  client.print(itoa(config.stripLen, buf, 10));
  reply(client, F(", moon: "));
  client.print(itoa(config.moonWidth, buf, 10));
  reply(client, F(", RGB: "));
  client.print(itoa(config.rgbR, buf, 10));
  client.print('/');
  client.print(itoa(config.rgbG, buf, 10));
  client.print('/');
  client.print(itoa(config.rgbB, buf, 10));
  reply(client, F("</p>"));
  reply(client, F("<form method='get' action='/setMoon'>"));  
  reply(client, F("Strip length <input name='strip-len' type='number' min='1' max='255' value='"));
  client.print(itoa(config.stripLen, buf, 10));
  reply(client, F("'></input>"));
  reply(client, F("<br/>Moon width <input name='moon-width' type='number' min='1' max='10'  value='"));
  client.print(itoa(config.moonWidth, buf, 10));
  reply(client, F("'></input>"));
  reply(client, F("<br/>Moon colour RGB"));
  reply(client, F("<input name='rgb-r' type='number' min='0' max='255' value='"));
  client.print(itoa(config.rgbR, buf, 10));
  reply(client, F("'></input>"));
  reply(client, F("<input name='rgb-g' type='number' min='0' max='255' value='"));
  client.print(itoa(config.rgbG, buf, 10));
  reply(client, F("'></input>"));
  reply(client, F("<input name='rgb-b' type='number' min='0' max='255' value='"));
  client.print(itoa(config.rgbB, buf, 10));
  reply(client, F("'></input>"));
  reply(client, F("</br><input name='set' value='Set Moon' type='submit'></input>"));
  reply(client, F("</form>"));  
  
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

const char goBack[] PROGMEM = R"=====(HTTP/1.1 303 See Other
Content-Type: text/html
Location: /
Connection: close

<!DOCTYPE html 
      PUBLIC "-//W3C//DTD HTML 4.01//EN"
      "http://www.w3.org/TR/html4/strict.dtd">
<html lang="en-US">
<head profile="http://www.w3.org/2005/10/profile">
<link rel="icon" href="/favicon.ico" />
<link rel="shortcut icon" href="/favicon.ico" />
<title>Fishtank Moon</title>
</head> 
<body>
<h1>Done</h1>
<p>
Done
</p>
)=====";

void WebserverPvt::setTime(WiFiClient &client) {

  
  
  reply(client, goBack);
}

void WebserverPvt::setSchedule(WiFiClient &client) {
  if(hasParam("moonrise")) {
    config.moonriseMins = getParamMins("moonrise");
  }
  if(hasParam("moonset")) {
    config.moonsetMins = getParamMins("moonset");
  }
  
  config.save();
  reply(client, goBack);
}

void WebserverPvt::setMoon(WiFiClient &client) {
  if(hasParam("strip-len")) {
    config.stripLen = getParamInt("strip-len");
  }
  if(hasParam("moon-width")) {
    config.stripLen = getParamInt("moon-width");
  }
  if(hasParam("rgb-r")) {
    config.rgbR = getParamInt("rgb-r");
  }
  if(hasParam("rgb-g")) {
    config.rgbG = getParamInt("rgb-g");
  }
  if(hasParam("rgb-b")) {
    config.rgbB = getParamInt("rgb-b");
  }

  config.save();
  reply(client, goBack);
}

