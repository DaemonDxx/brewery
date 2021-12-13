#ifndef brew_h
#define brew_h
#include "HeatController.h"
#include "Recipe.h"

class Brew
{
private:
    unsigned int _stage;
    unsigned long _start_stage_time;
    unsigned long _time_left;
    bool _is_Pause_Start;
    bool _is_On;
    HeatController *_controller;
    Recipe *_recipe;
public:
    Brew(HeatController *controller);
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