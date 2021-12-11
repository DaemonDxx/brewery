#ifndef Heater_h
#define Heater_h

#include "Arduino.h"

class Heater {
    public:
      Heater(unsigned int pin);
      void on();
      void off();
      bool isOn();
      void setPower(int percent);
      unsigned int getPower();
      void update();
    private:
      unsigned int _pin;
      bool _pin_state;
      bool _isOn;
      unsigned int _percent_power;
      unsigned int _acc;
};

#endif