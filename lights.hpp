#ifndef WFFT_LIGHTS
#define WFFT_LIGHTS 1

class Lights : public Runnable {
  public:
    void setup();
    void loop();
};

extern Lights lights;


#endif

