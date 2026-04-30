#include "Low/Outputs/output_LED.h"
#include "Low/config_hw.h"
#include "FastLED.h"

static CRGB LEDs_O[HW::LEDsCfg::LED_O_num];
static CRGB LEDs_X[HW::LEDsCfg::LED_X_num];

uint32_t _rgbToGrb(uint32_t rgb) {
    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >> 8) & 0xFF;
    uint8_t b = rgb & 0xFF;
    return ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
}

uint32_t _reduceBrightness(uint32_t color, float brightness) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >>  8) & 0xFF;
    uint8_t b = (color >>  0) & 0xFF;

    r = (uint8_t)(r * brightness);
    g = (uint8_t)(g * brightness);
    b = (uint8_t)(b * brightness);
    
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

void output_LED_init_() {
    FastLED.addLeds<WS2812B, HW::LEDsCfg::LEDSetO_Pin, RGB>(LEDs_O, HW::LEDsCfg::LED_O_num);
    FastLED.addLeds<WS2812B, HW::LEDsCfg::LEDSet_Xdebug_Pin, RGB>(LEDs_X, HW::LEDsCfg::LED_X_num);
    FastLED.setBrightness(OUT::led_O.brightness);
    FastLED.clear(true);
}


static void _output_LED_set_O() {
    LEDs_O[0] = _rgbToGrb(OUT::led_O.color);
    for (uint8_t i = 1; i < HW::LEDsCfg::LED_O_num; ++i) { //zapis do vsetkych O
        LEDs_O[i] = OUT::led_O.color;
    }
}


static void _output_LED_set_X() {
    for (uint8_t i = HW::LEDsCfg::LED_DEBUG_num; i < HW::LEDsCfg::LED_X_num - 1; ++i) { //zapis do vsetkych X
        LEDs_X[i] = OUT::led_X.color;
    }
}



static void _output_LED_set_DEBUG() {
    LEDs_X[0] = _rgbToGrb(_reduceBrightness(OUT::led_Debug.color, HW::LEDsCfg::DEBUG_BRIGHTNESS));
    for (uint8_t i = 1; i < HW::LEDsCfg::LED_DEBUG_num; ++i) { //zapis do vsetkych DEBUG
        LEDs_X[i] = _reduceBrightness(OUT::led_Debug.color, HW::LEDsCfg::DEBUG_BRIGHTNESS);
    }
}


void output_LED_update() {
    if(OUT::led_O.needsUpdate) { _output_LED_set_O(); OUT::led_O.needsUpdate = false;}
    if(OUT::led_X.needsUpdate) { _output_LED_set_X(); OUT::led_X.needsUpdate = false; }
    if(OUT::led_Debug.needsUpdate) { _output_LED_set_DEBUG(); OUT::led_Debug.needsUpdate = false; }


    Serial.println("llt");
    
    // FastLED.setBrightness(20);
    // fill_solid(LEDs_X, 20, CRGB::Red1);


    FastLED.show();
    //delay(100);

    // OUT::led_O.needsUpdate = false;
    // OUT::led_X.needsUpdate = false;
    // OUT::led_Debug.needsUpdate = false;

}



