#pragma once
#include "world.h"
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 display;

void output_display_init_();
void output_display_update();