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
static float line_Depth_Y = 0.0f;
static bool line_XoutOfReach = true;

static constexpr int dir_None = -1;
static constexpr int dir_North = 0;
static constexpr int dir_West = 1;
static constexpr int dir_South = 2;
static constexpr int dir_Eeast = 3;

static int8_t line_X_EncounterDir = dir_None;
static uint8_t line_angle_O_Active = 0;
static uint32_t line_O_lastSeenTime = 0;

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
        // line_Angle = (int16_t)round((atan2(angle_yCor, angle_xCor) * (180.0f / M_PI)) + 292.5f);
        line_Angle = (int16_t)round((atan2(angle_yCor, angle_xCor) * (180.0f / M_PI)) + 270.0f);
        line_Angle = wrap180(line_Angle);
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

        if(line_angle_O_Active == 0) { line_Angle = wrap180((line_X_CurrentDir * 90) + 180); }
        else { line_Angle = wrap180(line_Angle + 180); }

    } else {
        line_Depth = depth_currentMaxVal;
        if(line_angle_O_Active == 0) { line_Angle = wrap180(line_X_CurrentDir * 90); }
    }
}


static void _input_calculate_Depth_Y() {
    float frontMax = 0.0f;
    float backMax = 0.0f;

    // Vpredu: senzory 0..3, kde 3 je najdalej -> najvacsia hodnota
    for (uint8_t i = 0; i < 4; i++) {
        if (line_ADC_depth[i] <= HW::LineCfg::line_analog_treshold) {
            float val = HW::LineCfg::line_depth_XArr[3 - i];  // prehodene: 3=najvacsia
            if (val > frontMax) {
                frontMax = val;
            }
        }
    }

    // Vzadu: senzory 8..11, kde 11 je najdalej -> najvacsia hodnota
    for (uint8_t i = 8; i < 12; i++) {
        if (line_ADC_depth[i] <= HW::LineCfg::line_analog_treshold) {
            float val = HW::LineCfg::line_depth_XArr[3 - (i - 8)];  // prehodene: 11=najvacsia
            if (val > backMax) {
                backMax = val;
            }
        }
    }

    // Ak nic nevidim, nechaj poslednu hodnotu
    if (frontMax == 0.0f && backMax == 0.0f) {
        return;
    }

    if (frontMax > 0.0f && backMax > 0.0f) {
        line_Depth_Y = frontMax - backMax;
    }
    else if (frontMax > 0.0f) {
        line_Depth_Y = frontMax;    // +1.0 max vpredu
    }
    else {
        line_Depth_Y = -backMax;    // -1.0 max vzadu
    }
}

static bool line_O_outOfBounds = false;
static int8_t line_O_crossedSide = 0;
static int8_t line_O_latchedSide = 0;
static bool line_O_wasFullyOutside = false;
static bool line_O_startedReturn = false;

static void _input_calculate_OutOfBounds() {
    bool leftNow = false;
    bool centerNow = false;
    bool rightNow = false;

    for (uint8_t i = 14; i <= 15; i++) {
        if (line_ADC_ring[i] <= HW::LineCfg::line_analog_treshold) {
            leftNow = true;
            break;
        }
    }

    if (line_ADC_ring[16] <= HW::LineCfg::line_analog_treshold) {
        centerNow = true;
    }

    for (uint8_t i = 17; i <= 18; i++) {
        if (line_ADC_ring[i] <= HW::LineCfg::line_analog_treshold) {
            rightNow = true;
            break;
        }
    }

    bool seesA = leftNow || centerNow || rightNow;

    if (!line_O_outOfBounds && seesA) {
        line_O_outOfBounds = true;
        line_O_wasFullyOutside = false;
        line_O_startedReturn = false;

        if (leftNow && !rightNow) {
            line_O_latchedSide = -1;
        }
        else if (rightNow && !leftNow) {
            line_O_latchedSide = 1;
        }

        line_O_crossedSide = line_O_latchedSide;
        return;
    }

    if (line_O_outOfBounds) {
        line_O_crossedSide = line_O_latchedSide;

        if (!line_O_wasFullyOutside) {
            if (!seesA) {
                line_O_wasFullyOutside = true;
            }
            return;
        }

        if (line_O_wasFullyOutside && !line_O_startedReturn) {
            if (seesA) {
                line_O_startedReturn = true;
            }
            return;
        }

        if (line_O_wasFullyOutside && line_O_startedReturn) {
            if (!seesA) {
                line_O_outOfBounds = false;
                line_O_crossedSide = 0;
                line_O_latchedSide = 0;
                line_O_wasFullyOutside = false;
                line_O_startedReturn = false;
            }
            return;
        }
    }
}

static int16_t line_O_half0_Angle = 0;
static int16_t line_O_half1_Angle = 0;

static int16_t line_O_half0_Angle_ToNField = 0;
static int16_t line_O_half1_Angle_ToNField = 0;

static bool line_O_half0_Seen = false;
static bool line_O_half1_Seen = false;

static uint8_t line_O_half0_Count = 0;
static uint8_t line_O_half1_Count = 0;

static void _input_calculate_HalfAngles() {
    line_O_half0_Angle = 0;
    line_O_half1_Angle = 0;

    line_O_half0_Angle_ToNField = 0;
    line_O_half1_Angle_ToNField = 0;

    line_O_half0_Seen = false;
    line_O_half1_Seen = false;

    line_O_half0_Count = 0;
    line_O_half1_Count = 0;

    float half0_x = 0.0f;
    float half0_y = 0.0f;

    float half1_x = 0.0f;
    float half1_y = 0.0f;

    for (uint8_t i = 0; i < 16; i++) {
        if (line_ADC_ring[i] <= HW::LineCfg::line_analog_treshold) {
            line_O_half0_Count++;

            float ang = radians(i * 11.25f);
            half0_x += sinf(ang);
            half0_y += cosf(ang);
        }
    }

    for (uint8_t i = 16; i < 32; i++) {
        if (line_ADC_ring[i] <= HW::LineCfg::line_analog_treshold) {
            line_O_half1_Count++;

            float ang = radians(i * 11.25f);
            half1_x += sinf(ang);
            half1_y += cosf(ang);
        }
    }

    if (line_O_half0_Count > 0) {
        line_O_half0_Seen = true;
        line_O_half0_Angle = wrap180((int16_t)roundf(degrees(atan2f(half0_x, half0_y))));
        line_O_half0_Angle_ToNField = wrap180(line_O_half0_Angle - IN::gyro.offset);
    }

    if (line_O_half1_Count > 0) {
        line_O_half1_Seen = true;
        line_O_half1_Angle = wrap180((int16_t)roundf(degrees(atan2f(half1_x, half1_y))));
        line_O_half1_Angle_ToNField = wrap180(line_O_half1_Angle - IN::gyro.offset);
    }
}



static bool line_O_outside = false;
static int8_t line_O_outsideSide = 0;   // -1 = dolava, 1 = doprava, 0 = nic
static uint16_t line_O_halfDiffAbs = 0; // error = | |A0| - |A1| |

static uint8_t line_O_outsideOnCnt = 0;
static uint8_t line_O_outsideOffCnt = 0;

static void _input_calculate_OutsideFromHalfAngles() {
    if (!line_O_half0_Seen || !line_O_half1_Seen) {
        line_O_outsideOnCnt = 0;
        line_O_outsideOffCnt = 0;
        line_O_outside = false;
        line_O_outsideSide = 0;
        line_O_halfDiffAbs = 0;
        return;
    }

    int16_t A0 = line_O_half0_Angle_ToNField;
    int16_t A1 = line_O_half1_Angle_ToNField;

    uint16_t error = abs(abs(A0) - abs(A1));
    line_O_halfDiffAbs = error;

    const uint16_t OUTSIDE_ON_THR  = 50;
    const uint16_t OUTSIDE_OFF_THR = 35;

    const uint8_t OUTSIDE_ON_SAMPLES  = 3;
    const uint8_t OUTSIDE_OFF_SAMPLES = 3;

    int8_t sideNow = 0;
    if (abs(A0) > abs(A1)) {
        sideNow = -1;
    }
    else if (abs(A1) > abs(A0)) {
        sideNow = 1;
    }

    if (!line_O_outside) {
        if (error >= OUTSIDE_ON_THR) {
            if (line_O_outsideOnCnt < 255) line_O_outsideOnCnt++;
        } else {
            line_O_outsideOnCnt = 0;
        }

        if (line_O_outsideOnCnt >= OUTSIDE_ON_SAMPLES) {
            line_O_outside = true;
            line_O_outsideSide = sideNow;
            line_O_outsideOnCnt = 0;
            line_O_outsideOffCnt = 0;
        }
    }
    else {
        if (error <= OUTSIDE_OFF_THR) {
            if (line_O_outsideOffCnt < 255) line_O_outsideOffCnt++;
        } else {
            line_O_outsideOffCnt = 0;

            // ked uz som outside, stranu mozes priebezne obnovovat
            if (sideNow != 0) {
                line_O_outsideSide = sideNow;
            }
        }

        if (line_O_outsideOffCnt >= OUTSIDE_OFF_SAMPLES) {
            line_O_outside = false;
            line_O_outsideSide = 0;
            line_O_outsideOnCnt = 0;
            line_O_outsideOffCnt = 0;
        }
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

        
    if(ST::state.isRobot_A == false) {
        // IN::line.depth_Y = false;
        _input_calculate_Depth();
    }
    
    

    IN::line.angle = line_Angle;
    IN::line.depth = line_Depth;
    IN::line.angle_ToNField = wrap360(line_Angle - IN::gyro.offset);
    IN::line.seen = (line_Depth != 0.0f);

    if(IN::line.seen) {
        IN::line.lastSeen_ms = ST::now_ms;
    }
    


    if(ST::soccer.role == ST::Types::role_t::Defender) {
        _input_calculate_Depth_Y();
        // _input_calculate_OutOfBounds();
        // _input_calculate_Curvature();
        _input_calculate_HalfAngles();
        _input_calculate_OutsideFromHalfAngles();
        
        IN::line.defender_outOfBounds = line_O_outside;
        // IN::line.defender_crossedSide = -1*(line_O_crossedSide - 2);
        IN::line.defender_crossedSide = line_O_outsideSide;


        IN::line.depth_Y = line_Depth_Y;

    #if DEBUG_LINE_DEFENDER
    Serial.printf("A0:%d A1:%d Err:%u Out:%d Side:%d OnCnt:%u OffCnt:%u\n",
        A0,
        A1,
        line_O_halfDiffAbs,
        line_O_outside,
        line_O_outsideSide,
        line_O_outsideOnCnt,
        line_O_outsideOffCnt
    );
    #endif

    }


    #if DEBUG_LED_LINE
        if(debug_line_LED_enabled != IN::line.seen) {
            debug_line_LED_enabled = IN::line.seen;
            OUT::led_Debug.needsUpdate = true;
            if(IN::line.seen == true) {
                OUT::led_Debug.color[0] = CRGB::DarkRed;
            } else {
                OUT::led_Debug.color[0] = CRGB::Black;
            }
        }
    #endif

    #if DEBUG_DISPLAY_LINE
        if(ST::now_ms - debug_line_last_display_time >= 100) {
            if(ST::state.mode != ST::Types::mode_t::Off && ST::state.mode == ST::state.mode_previous){
                debug_line_last_display_time = ST::now_ms;
                OUT::display.needsUpdate = true;
                display.setTextSize(1);
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
                display.print(IN::line.angle);
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


                display.setCursor(78, 53);
                display.print("Y:");
                display.setCursor(90, 53);
                display.print(line_Depth_Y);
            }
        }

    #endif

    #if DEBUG_LINE_ANG
        Serial.printf("Ang: %0.2f", IN::line.angle_ToNField);
        Serial.printf(" || Depth: %0.2f \n", IN::line.depth);
    #endif
}








