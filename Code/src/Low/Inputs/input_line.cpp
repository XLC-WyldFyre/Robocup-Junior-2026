#include "Low/Inputs/input_line.h"
#include "Low/config_hw.h"
#include "low/inputs.h"
#include "driver/gpio.h"
#include <Arduino.h>
#include <FastADC_Mux.h>
#include "High/Services/debug.h"
#include "Low/Outputs/output_display.h"


static uint16_t line_ADC_ring[32];
static uint16_t line_ADC_depth[16]; 

static int16_t line_Angle = 0;
static float line_Depth = 0.0f;
static bool line_XoutOfReach = true;

static constexpr int dir_None = -1;
static constexpr int dir_North = 0;
static constexpr int dir_West = 1;
static constexpr int dir_South = 2;
static constexpr int dir_Eeast = 3;

static int8_t line_X_EncounterDir = dir_None;
static uint8_t line_angle_O_Active = 0;

#if DEBUG_LED_LINE
    bool debug_line_LED_enabled = false; 
    uint32_t debug_line_last_display_time = 0;
#endif


static void _input_setMuxAddr(uint8_t a) {
    gpio_set_level(HW::LineCfg::LineSet0_Pin, a & 1);
    gpio_set_level(HW::LineCfg::LineSet1_Pin, (a >> 1) & 1);
    gpio_set_level(HW::LineCfg::LineSet2_Pin, (a >> 2) & 1);
    gpio_set_level(HW::LineCfg::LineSet3_Pin, (a >> 3) & 1);
}



static void _input_scanMux() {
    for(uint8_t addr = 0; addr < 16; addr++) {

        _input_setMuxAddr(addr);

        line_ADC_ring[addr] = analogReadFast(HW::LineCfg::Line_A_Read_ADC_CH);
        line_ADC_ring[addr+16] = analogReadFast(HW::LineCfg::Line_B_Read_ADC_CH);
        line_ADC_depth[addr] = analogReadFast(HW::LineCfg::Line_X_Read_ADC_CH);

    }

        #if DEBUG_LINE_RAW_RING

        Serial.print("Ring: ");
        for(int i = 0; i<32; i++) {
            Serial.print("|");
            Serial.print(i);
            Serial.print(": ");
            Serial.print(line_ADC_ring[i]);
        }
        Serial.println("");
        #endif

        #if DEBUG_LINE_RAW_DEPTH
        Serial.print("Depth: ");
        for(int i = 0; i<16; i++) {
            Serial.print("|");
            Serial.print(i);
            Serial.print(": ");
            Serial.print(line_ADC_depth[i]);
        }
        Serial.println("");
        #endif
}


static void _input_calculate_Angle() {
    line_angle_O_Active = 0;
    float angle_xCor = 0, angle_yCor = 0;
    for(uint8_t i = 0; i < 32 ; i++) {
        if(line_ADC_ring[i] <= HW::LineCfg::line_analog_treshold) {
            line_angle_O_Active++;
            angle_xCor += sin(radians(i * 11.25f));  //11.25deg per sensor
            angle_yCor += cos(radians(i * 11.25f));
        }
    }
    if(line_angle_O_Active != 0) { 
        line_Depth = HW::LineCfg::line_depth_O_Val; 
        line_Angle = (int16_t)round((atan2(angle_yCor, angle_xCor) * (180.0f / M_PI)) + 292.5f);
        line_Angle = input_wrap180(line_Angle);
    }

    else { 
        line_Depth = 0.0f;
    }
}



static void _input_calculate_Depth() {
    float depth_currentMaxVal = 0.0f;
    int8_t line_X_CurrentDir = dir_None;

    for(uint8_t i = 0; i < 16; i++) { 
        if(line_ADC_depth[i] <= HW::LineCfg::line_analog_treshold) { //sensor see line 
            float depth_pom_val = HW::LineCfg::line_depth_XArr[i%4];
            if(depth_pom_val > depth_currentMaxVal) { //bigger val caounts
                depth_currentMaxVal = depth_pom_val;
                line_X_CurrentDir = (int8_t)(i/4);
            }
        }
    }

    if(depth_currentMaxVal == 0.0f) { // X line sensors not on the line
        line_XoutOfReach = true;
        line_X_EncounterDir = dir_None;
        return;
    }

    if(line_XoutOfReach == true) { //X line sens on line and first encounter 
        line_XoutOfReach = false;
        line_X_EncounterDir = line_X_CurrentDir;
    }

    if(line_X_CurrentDir == (line_X_EncounterDir ^ 2)) { //XOR ^2 flips direction
        line_Depth = 1.0f - depth_currentMaxVal;

        if(line_angle_O_Active == 0) { line_Angle = input_wrap180((line_X_CurrentDir * 90) + 180); }
        else { line_Angle = input_wrap180(line_Angle + 180); }

    } else {
        line_Depth = depth_currentMaxVal;
        if(line_angle_O_Active == 0) { line_Angle = input_wrap180(line_X_CurrentDir * 90); }
    }
}



void input_line_init_() {
    gpio_set_direction(HW::LineCfg::LineSet0_Pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(HW::LineCfg::LineSet1_Pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(HW::LineCfg::LineSet2_Pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(HW::LineCfg::LineSet3_Pin, GPIO_MODE_OUTPUT);

    fadcInit(3, HW::LineCfg::Line_A_Read_Pin, HW::LineCfg::Line_B_Read_Pin, HW::LineCfg::Line_X_Read_Pin);
}


void input_line_update() {

    _input_scanMux();

    _input_calculate_Angle();
    _input_calculate_Depth();
    IN::line.angle = line_Angle;
    IN::line.depth = line_Depth;
    IN::line.angle_ToNField = input_wrap180(line_Angle - IN::gyro.offset);
    IN::line.seen = (line_Depth != 0.0f);

    #if DEBUG_LED_LINE
        if(debug_line_LED_enabled != IN::line.seen) {
            debug_line_LED_enabled = IN::line.seen;
            OUT::led_Debug.needsUpdate = true;
            if(IN::line.seen == true) {
                OUT::led_Debug.color = CRGB::DarkRed;
            } else {
                OUT::led_Debug.color = CRGB::DarkGreen;
            }
        }
    #endif

    #if DEBUG_DISPLAY_LINE
        if(ST::now_ms - debug_line_last_display_time >= 100) {
            if(ST::state.mode != ST::Types::mode_t::Off && ST::state.mode == ST::state.mode_previous){
                debug_line_last_display_time = ST::now_ms;
                OUT::display.needsUpdate = true;
                display.setTextColor(1);
                display.setTextWrap(false);
                display.setCursor(2, 21);
                display.print("Angle:");

                display.setCursor(3, 53);
                display.print("Depth:");

                display.setCursor(3, 37);
                display.print("Num(O):");

                display.setCursor(37, 21);
                if(IN::line.seen) {
                display.print(line_Angle);
                } else {
                    display.print("-");
                }

                display.setCursor(41, 53);
                if(IN::line.seen) {
                    display.print(line_Depth);
                } else {
                    display.print("-");
                }

                display.setCursor(46, 37);
                display.print(line_angle_O_Active);

                display.setCursor(2, 5);
                display.print(">Line<");
            }
        }

    #endif

    #if DEBUG_LINE_ANG
        Serial.print("Ang: " +  IN::line.angle);
        Serial.printf(" || Depth: &0.2f", IN::line.depth);
    #endif
}