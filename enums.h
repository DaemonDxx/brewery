#ifndef enums_h
#define enums_h

enum BEEP_MODE { 
    START,
    VERBOSE,
    ERROR ,
    NEXT_STAGE,
    END,
    PAUSE_START
};

enum HEAT_MODE {
    HEATING,
    RETENTION,
    COOLING
};

enum CHANGE_PARAM {
    TEMPERATURE,
    DELAY
};

#endif