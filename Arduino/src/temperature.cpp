#include "pins.h"
#include <Arduino.h>

#include <DallasTemperature.h>
#include <OneWire.h>

OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

void init_temp()
{
    sensors.begin();
}

float getTemperature()
{
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    return tempC;
}
