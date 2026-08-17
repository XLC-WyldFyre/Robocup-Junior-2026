#pragma once

#include "world.h"


namespace OUT {
        uint32_t LedReduceBrightness(uint32_t color, float brightness);
}

void output_LED_init_();


void output_LED_update();