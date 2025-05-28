#include "pins.h"
#include <Arduino.h>

float getPhValue()
{
    int rawPH = analogRead(PH_SENSOR_PIN);
    float voltagePH = rawPH * 5.0 / 1023.0;
    float pHValue = 7 + ((1.9 - voltagePH) / 0.18);

    return pHValue;
}
