#pragma once

float getPhValue();

void init_temp();
float getTemperature();

float getTurbidity();

void init_waterlvl();
int getWaterLevel();

void init_LEDs();
void setLEDsIntensity(int intensity);
int getLEDsBrightness();

void init_light();
float getLightValue();
void led_loop();
