#include "config.h"
#include "pins.h"
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
static int brightness = 0; // Brightness level from 0 to 255

void init_LEDs()
{
    strip.begin();
    for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(255, 147, 41)); // warm white
    }
    strip.setBrightness(brightness);
    strip.show();
}

void setLEDsIntensity(int intensity)
{
    brightness = intensity;
    strip.setBrightness(brightness);
    for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(255, 147, 41)); // Reapply warm white
    }
    strip.show();
}

void led_loop()
{
    strip.show();
}

int getLEDsBrightness()
{
    return brightness;
}
