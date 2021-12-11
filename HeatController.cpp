#include "HeatController.h"

#define Kp 2
#define Ki 5
#define Kd 1

HeatController::HeatController(Heater *heat_weak, Heater *heat_strong) {
    _heat_weak = heat_weak;
    _heat_strong = heat_strong;
    PID pid(&_current_temperature, &_current_power, &_setTemperature, Kp, Ki, Kd, DIRECT);
    pid.SetOutputLimits(0, 100);
    // Сделать когда-нибудь по-человечески
    pid.SetSampleTime(150);
    _pid = &pid;
}

void HeatController::On() {
    _heat_strong -> on();
    _heat_weak -> on();
}

void HeatController::Off() {
    _heat_strong -> off();
    _heat_weak -> off();
}

void HeatController::SetTemperature(unsigned int temp) {
    _current_temperature = temp;
}

void HeatController::Update(float current_temp) {
}

