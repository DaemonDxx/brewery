#ifndef __HEATCONTROLLER_H__
#define __HEATCONTROLLER_H__

#include "Arduino.h"
#include "enums.h"
#include "Heater.h"
#include <PID_v1.h>
#include "HeaterGroup.h"

#define HEATER_FULL_INTERVAL_UPDATE 15000
#define HEATER_CONTROLLER_INTERVAL 1000
#define TEMPERATURE_WINDOW 1.0

class HeatController
{
private:
    double _current_power;
    double _current_temperature;
    double _setTemperature;
    uint8_t _max_power;
    bool _isOn;
    HeaterGroup *_heater;
    PID *_pid;
public:
    HeatController(HeaterGroup *heater);
    void setTemperature(unsigned int temp);
    void setMaxPower(unsigned int max_power);
    double getCurrentTemperature();
    unsigned long getCurrentPower();
    void on();
    void off();
    bool isOn();
    void update(double current_temp);
};

#endif // __HEATCONTROLLER_H__