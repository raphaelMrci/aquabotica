#include "pins.h"
#include "waterlevelsensor.h"
#include <Arduino.h>

WaterLevelSensor sensor = WaterLevelSensor();

void init_waterlvl()
{
    // Dummy reads to trigger sensor startup
    sensor.readPercentage();
    delay(100);
    sensor.readPercentage();
}

int getWaterLevel()
{
    int waterLevel = sensor.readPercentage();

    return waterLevel;
}
