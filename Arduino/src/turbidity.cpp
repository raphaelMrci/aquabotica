#include "pins.h"
#include <Arduino.h>

float getTurbidity()
{
    int rawValue = analogRead(TURB_SENSOR_PIN);
    float voltage = rawValue * (5.0 / 1023.0);
    float ntu = -1120.4 * voltage * voltage + 5742.3 * voltage - 4352.9;

    return ntu;
}
