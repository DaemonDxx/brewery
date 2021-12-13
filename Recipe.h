#ifndef Recipe_h
#define Recipe_h
class Recipe
{
private:
    unsigned int _stages;
    unsigned long *_pauses;
    unsigned int *_temperatures;
public:
    Recipe(unsigned int stage_count);
    ~Recipe();
    void setStage(unsigned int stage, unsigned long pause, unsigned int temperature);
    unsigned int getTemperature(unsigned int stage);
    unsigned int getPause(unsigned int stage);
    unsigned int getStageCount();
};

#endif