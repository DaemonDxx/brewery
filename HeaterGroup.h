#ifndef HeaterGroup_h
#define HeaterGroup_h

#include "Heater.h"

class HeaterGroup
{
private:
    Heater *_week;
    Heater *_strong;
    bool _isOn;
    unsigned int _power;
public:
    HeaterGroup(Heater *week, Heater *strong);
    void setPower(int power);
    void on();
    void off();
    unsigned int getPower();
    bool isOn();
    void update();
};


#endif