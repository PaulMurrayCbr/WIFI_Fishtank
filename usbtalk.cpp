#include <Arduino.h>
#include <EEPROM.h>

#include "base.hpp"
#include "usbtalk.hpp"
#include "config.hpp"

class UsbTalkPvt {
    friend UsbTalk;
    char usbCmd;
    char usbLn[256];
    int usbLnCt;

    enum State {
      CMD,
      ARGUMENT
    } state = CMD;

    enum Command {
      SSID, PASSWORD
    }
    command;

    void msg() {
      Serial.println(F("Ready to configure"));
      Serial.print(F("Current SSID: "));
      Serial.println(config.ssid);
      Serial.print(F("Current password: "));
      Serial.println(config.password);
      Serial.println(F("S - enter SSID"));
      Serial.println(F("P - enter Password"));
      Serial.println(F("? - this message"));
    }

    void setup() {
#ifndef DEBUG
      // We can't wait for serial, because there may be no USB connected
      Serial.begin(115200); // initializing Serial
      delay(500);
#endif

      if (!Serial) return;

      msg();

    }

    void loop() {
      if (!Serial || !Serial.available()) return;

      static char was = '\0';
      char ch = Serial.read();

      if ((ch == '\n' && was == '\r') || (ch == '\r' && was == '\n')) {
        was = '\0';
        return;
      }


      if (ch >= ' ' && ch < 127) {
        if (usbLnCt < sizeof(usbLn) - 1) {
          usbLn[usbLnCt++] = ch;
          usbLn[usbLnCt] = '\0';
        }
        was = '\0';
      }
      else if (ch == '\n' || ch == '\r') {
        LOGN(F("Entered: "));
        LOG(usbLn);


        switch (state) {
          case CMD:
            switch (usbLn[0]) {
              case 's': case 'S':
                Serial.print(F("New SSID: "));
                command = SSID;
                state = ARGUMENT;
                break;
              case 'p': case 'P':
                Serial.print(F("New Password: "));
                command = PASSWORD;
                state = ARGUMENT;
                break;
              default:
                msg();
                break;
            }
            break;

          case ARGUMENT:
            switch (command) {
              case SSID:
                strncpy(config.ssid, usbLn, sizeof(config.ssid));
                config.ssid[sizeof(config.ssid) - 1] = '\0';
                config.save();
                state = CMD;
                break;
              case PASSWORD:
                strncpy(config.password, usbLn, sizeof(config.password));
                config.ssid[sizeof(config.password) - 1] = '\0';
                config.save();
                state = CMD;
                break;
            }
            break;
        }
        usbLn[0] = '\0';
        usbLnCt = 0;
        was = ch;
      }
    }

} usbTalkPvt;

UsbTalk usbTalk;


void UsbTalk::setup() {
  usbTalkPvt.setup();
}

void UsbTalk::loop() {
  usbTalkPvt.loop();

}


