#ifndef WFFT_CONFIG
#define WFFT_CONFIG 1

class Config {
    static const uint32_t MAGIC_NUMBER = 0xF00D002;
    uint32_t magic_number;

  public:
    void setup();
    void save();

    char ssid[20] = "Unit 61"; // type your ssid
    char password[20] = "Bing.zal.mir3"; // type your password

};

extern Config config;


#endif
