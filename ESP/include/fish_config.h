#ifndef FISH_CONFIG_H
#define FISH_CONFIG_H

#include <Arduino.h>

typedef struct fish_config_s {
    String name;
    unsigned int max_temp;
    unsigned int min_temp;
    float max_ph;
    float min_ph;
    unsigned int max_turb;
    unsigned int min_turb;
    unsigned int max_light;
    unsigned int min_light;
    unsigned int min_water_lvl;
} fish_config_t;

typedef enum fishes_e {
    NONE = 0,
    BLUEDORY = 1,
    PONYO = 2,
    DISCOBASS = 3,
} fishes_t;

extern fish_config_t fish_config[];

#define FISH_COUNT 4

#endif
