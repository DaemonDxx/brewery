#include "Recipe.h"

Recipe::Recipe(unsigned int stage_count) {
    _stages = stage_count;
    _pauses = new unsigned long[stage_count];
    _temperatures = new unsigned int[stage_count];
}

Recipe::~Recipe() 
{
    delete[] _pauses;
    delete[] _temperatures;
}

void Recipe::setStage(unsigned int stage, unsigned long pause, unsigned int temperature) {
    _pauses[stage] = pause;
    _temperatures[stage] = temperature;
}

unsigned int Recipe::getTemperature(unsigned int stage) 
{
    return _temperatures[stage]; 
}

unsigned int Recipe::getPause(unsigned int stage) 
{
    return _pauses[stage];
}

unsigned int Recipe::getStageCount() 
{
    return _stages;
}
