#include "fish_config.h"

fish_config_t fish_config[] = {
    {

        // NONE
        .name = "NONE",
        .max_temp = 0,
        .min_temp = 0,
        .max_ph = 0,
        .min_ph = 0,
        .max_turb = 0,
        .min_turb = 0,
        .max_light = 0,
        .min_light = 0,
        .min_water_lvl = 0,
    },
    {
        // FISH_1
        .name = "bluedory",
        .max_temp = 30,
        .min_temp = 15,
        .max_ph = 8,
        .min_ph = 6,
        .max_turb = 1000,
        .min_turb = 2500,
        .max_light = 1000,
        .min_light = 0,
        .min_water_lvl = 70,
    },
    {
        // FISH_2
        .name = "unknown_fish",
        .max_temp = 25,
        .min_temp = 10,
        .max_ph = 7.8,
        .min_ph = 6.5,
        .max_turb = 900,
        .min_turb = 2500,
        .max_light = 800,
        .min_light = 200,
        .min_water_lvl = 30,
    },
    {
        // FISH_3
        .name = "discobass",
        .max_temp = 28,
        .min_temp = 10,
        .max_ph = 7.5,
        .min_ph = 6.8,
        .max_turb = 2000,
        .min_turb = 2500,
        .max_light = 950,
        .min_light = 100,
        .min_water_lvl = 50,
    },
};
