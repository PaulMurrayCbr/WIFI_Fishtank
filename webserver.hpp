#ifndef WFFT_WEBSERVER
#define WFFT_WEBSERVER 1

#import "config.hpp"

class Webserver  : public Runnable, ConfigListener  {
    void setup();
    void loop();
    void configChanged();
};

extern Webserver webserver;

#endif
