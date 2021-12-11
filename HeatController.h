#include "Arduino.h"
#include "enums.h"
#include "Heater.h"

#define HEATER_FULL_INTERVAL_UPDATE 15000
#define HEATER_CONTROLLER_INTERVAL 1000
#define TEMPERATURE_WINDOW 1.0

class HeatController
{
private:
    double _current_power;
    double _current_temperature;
    double _setTemperature;
    Heater *_heat_weak;
    Heater *_heat_strong;
    //PID *_pid;
public:
    HeatController(Heater *heat_weak, Heater *heat_strong);
    void SetTemperature(unsigned int temp);
    void On();
    void Off();
    void Update(float current_temp);
};
