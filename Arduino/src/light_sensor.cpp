#include "pins.h"
#include <Arduino.h>

void init_light()
{
    pinMode(LDR_PIN, INPUT);
}

float getLightValue()
{
    int rawValue = analogRead(LDR_PIN);
    return rawValue;
}
