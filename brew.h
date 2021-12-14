#ifndef brew_h
#define brew_h
#include "HeatController.h"
#include "Recipe.h"
#include "enums.h"

typedef void (*MakeSoundFunc) (BEEP_MODE sound);

class Brew
{
private:
    unsigned int _stage;
    unsigned long _end_time_stage;
    unsigned long _time_left;
    MakeSoundFunc _buzzer;
    bool _is_Pause_Start;
    bool _is_On;
    HeatController *_controller;
    Recipe *_recipe;
public:
    Brew(HeatController *controller, MakeSoundFunc buzzer);
    void setRecipe(Recipe *recipe);
    Recipe* getRecipe();
    void start();
    void nextStage();
    void cancel();
    unsigned int getCurrentStage();
    unsigned int getTimeLeft();
    void update();
};

#endif