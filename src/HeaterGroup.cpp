#include "HeaterGroup.h"

HeaterGroup::HeaterGroup(Heater *week, Heater *strong) {
    _week = week;
    _strong = strong;
    _isOn = false;
    week -> off();
    strong -> off();
}

bool HeaterGroup::isOn() {
    return _isOn;
}

void HeaterGroup::off() {
    _week -> off();
    _strong -> off();
    _isOn = false;
}

void HeaterGroup::on() {
    _week -> on();
    _strong -> on();
    _isOn = true;
}

unsigned int HeaterGroup::getPower() {
    return _power;
}

void HeaterGroup::update() {
    _week -> update();
    _strong -> update();
}

void HeaterGroup::setPower(int power) {
    unsigned int weekPower = 0;
    unsigned int strongPower = 0;
    if (power <= 0) {
        _power = 0;
    } else if (power > 0 && power <= 33) {
        weekPower = round(power*100/33);
    } else if (power > 33 && power <= 100) {
        weekPower = 100;
        strongPower = round(((power - 33) *100 / 67));
    } else {
        weekPower = 100;
        strongPower = 100;
    }
    _week -> setPower(weekPower);
    _strong -> setPower(strongPower);
}