#include "HeatController.h"

#define Kp 20
#define Ki 20
#define Kd 10

#define POWER_COEFF 30

HeatController::HeatController(HeaterGroup *heater) {
    _isOn = false;
    _heater = heater;
    _pid = new PID(&_current_temperature, &_current_power, &_setTemperature, Kp, Ki, Kd, DIRECT);
    _pid -> SetOutputLimits(0, 100);
    _pid -> SetSampleTime(2000);
    _pid -> SetMode(AUTOMATIC);
}

void HeatController::on() {
    _heater->on();
    _isOn = true;
}

void HeatController::off() 
{
    _heater -> off();
    _isOn = false;
}

void HeatController::setTemperature(unsigned int temp) {
    _setTemperature = temp;
}

double HeatController::getCurrentTemperature() 
{
    return _current_temperature;
}

unsigned long HeatController::getCurrentPower() 
{   
    return _current_power * POWER_COEFF;
}


void HeatController::update(double current_temp) {
    if (_isOn) {
        _current_temperature = current_temp;
    _pid->Compute();
    _heater -> setPower(_current_power);
    Serial.print(_setTemperature);
    Serial.print(" ");
    Serial.print(_current_temperature);
    Serial.print(" ");
    Serial.println(_current_power);
    }
}

bool HeatController::isOn() {
    return _isOn;
}

