#include "Brew.h"
#define TEMPERATURE_WINDOW -1

Brew::Brew(HeatController *controller) 
{
    _controller = controller;
    _is_On = false;
    _stage = 0;
}

void Brew::setRecipe(Recipe *recipe) 
{
    _recipe = recipe;
}

Recipe* Brew::getRecipe() 
{
    return _recipe;
}

void Brew::start() 
{
    _stage = -1;
    _controller -> setTemperature(0);
    _controller -> on();
    _is_On = true;
    this -> nextStage();
}

void Brew::nextStage() 
{
    if ((_recipe -> getStageCount() - 1) == _stage) {
        this -> cancel();
    } else {
         _stage++;
        _is_Pause_Start = false;
        _time_left = _recipe -> getPause(_stage);
        _controller -> setTemperature(_recipe -> getTemperature(_stage));
        _end_time_stage = 0;
        this -> update();
    }
}

void Brew::cancel() 
{
    _stage = -1;
    _controller -> off();
    _controller -> setTemperature(0);
    _is_On = false;
}

unsigned int Brew::getCurrentStage() 
{
    return _stage;
}

unsigned int Brew::getTimeLeft() 
{
    if (_is_Pause_Start) {
        return _time_left;
    } else {
        return _recipe -> getPause(_stage);
    }
}

void Brew::update() 
{
    if (_is_On) {
        if (_is_Pause_Start) {
            _time_left = - (millis()/1000 - _end_time_stage);
            if (_time_left  <= 0) {
                this -> nextStage();
            }
        } else {
            if (_controller -> getCurrentTemperature() - _recipe -> getTemperature(_stage) >= TEMPERATURE_WINDOW) {
                _is_Pause_Start = true;
                _end_time_stage = millis()/1000 + _recipe -> getPause(_stage);
                Serial.println(millis()/1000);
                Serial.println(_end_time_stage);
                Serial.println("Pause starts");
            } 
        }
    }
}
