#ifndef WFFT_CONFIG
#define WFFT_CONFIG 1

class Config {
    static const uint32_t MAGIC_NUMBER = 0xF00D003;
    uint32_t magic_number;

  public:
    void setup();
    void save();

    char ssid[20] = "Unit 61"; // type your ssid
    char password[20] = "Bing.zal.mir3"; // type your password

    int moonriseMins;
    int moonsetMins;

    int stripLen;
    int moonWidth;

    byte rgbR;
    byte rgbG;
    byte rgbB;
};

class ConfigListener {
    static ConfigListener *headConfigListener;
    ConfigListener *nextConfigListener;

  protected:
    virtual void configChanged() = 0;

  public:
    ConfigListener() {
      nextConfigListener = headConfigListener;
      headConfigListener = this;
    }

    static void notifyAll() {
      for (ConfigListener *r = headConfigListener; r; r = r->nextConfigListener)
        r->configChanged();
    }

};


extern Config config;


#endif
