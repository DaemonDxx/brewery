#ifndef brew_h
#define brew_h
#include "HeatController.h"


class Brew
{
private:
    unsigned int _stage;
public:
    Brew(HeatController *controller);
    void setRecipe(unsigned int *pauses, unsigned int);
};



#endif