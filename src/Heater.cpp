#include "Heater.h"

Heater::Heater(unsigned int pin) {
  _pin = pin;
  _isOn = false;
  _percent_power = 0;
  _acc = 0;
  _pin_state = LOW;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, _pin_state);
}

void Heater::on() {
  _isOn = true;
}

void Heater::off() {
  _isOn = false;
  _pin_state = LOW;
  digitalWrite(_pin, _pin_state);
}

bool Heater::isOn() {
  return _isOn;
}

void Heater::setPower(int percent) {
  if (percent <= 0) {
    _percent_power = 0;
    _pin_state = 0;
    digitalWrite(_pin, _pin_state);
  } else if (percent > 100) {
    _percent_power = 100;
  } else {
    _percent_power = percent;
  }
}

unsigned int Heater::getPower() {
  return _percent_power;
}

void Heater::update() {
  _pin_state = LOW;
  _acc += _percent_power;
  if (_acc >= 100 && _isOn) {
    _pin_state = HIGH;
    _acc -= 100;
  }
  digitalWrite(_pin, _pin_state);
}