#ifndef WFFT_USBTALK
#define WFFT_USBTALK 1

class UsbTalk: public Runnable {
  public:
    void setup();
    void loop();

};

extern UsbTalk usbTalk;


#endif
