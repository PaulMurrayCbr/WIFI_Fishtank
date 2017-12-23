#ifndef WFFT_BASE
#define WFFT_BASE 1

#define DEBUG 1

#ifdef DEBUG

#define LOG(s) Serial.println(s)
#define LOGN(s) Serial.print(s)

#else

#define LOG(s)
#define LOGN(s)

#endif

#endif

class Runnable {
    static Runnable *headRunnable;
    Runnable *nextRunnable;

    virtual void setup() = 0;
    virtual void loop() = 0;

  public:
    Runnable() {
      nextRunnable = headRunnable;
      headRunnable = this;
    }

    static void setupAll() {
      for (Runnable *r = headRunnable; r; r = r->nextRunnable)
        r->setup();
    }

    static void loopAll() {
      for (Runnable *r = headRunnable; r; r = r->nextRunnable)
        r->loop();
    }

};


