#include "Low/Inputs/input_motionSense.h"
#include "driver/gpio.h"
#include "Low/config_hw.h"
#include <FastADC_Mux.h>
#include "High/Services/debug.h"

static bool speeds[4]; //5

static constexpr float motion_ADC_Const = 1.0f/HW::MotionSenseCfg::ADC_maxVal;

#if DEBUG_LED_HASBALL
    bool debug_ball_LED_enabled = false; 
#endif

void _input_motionSense_readSpeed() {
    // speeds_ADC[0] = analogReadFast(HW::MotionSenseCfg::A_Spd_Pin_ADC_CH);
    // speeds_ADC[1] = analogReadFast(HW::MotionSenseCfg::B_Spd_Pin_ADC_CH);
    // speeds_ADC[2] = analogReadFast(HW::MotionSenseCfg::C_Spd_Pin_ADC_CH);
    // speeds_ADC[3] = analogReadFast(HW::MotionSenseCfg::D_Spd_Pin_ADC_CH);
    // speeds_ADC[motion_motDRBRF] = 

}

void input_motionSense_init_() {
    gpio_set_direction(HW::MotionSenseCfg::DRB_SW_F_Pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(HW::MotionSenseCfg::DRB_SW_F_Pin, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(HW::MotionSenseCfg::DRB_SW_R_Pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(HW::MotionSenseCfg::DRB_SW_R_Pin, GPIO_PULLDOWN_ONLY);

    gpio_set_direction(HW::MotionSenseCfg::A_Spd_Pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(HW::MotionSenseCfg::A_Spd_Pin, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(HW::MotionSenseCfg::B_Spd_Pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(HW::MotionSenseCfg::B_Spd_Pin, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(HW::MotionSenseCfg::C_Spd_Pin, GPIO_MODE_INPUT);
gpio_set_pull_mode(HW::MotionSenseCfg::C_Spd_Pin, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(HW::MotionSenseCfg::D_Spd_Pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(HW::MotionSenseCfg::D_Spd_Pin, GPIO_PULLDOWN_ONLY);
    

    // speeds_ADC[motion_motDRBRF] = analogReadFast(motion_CH_DRBFR);

    if(speeds[0] == 1) {
        ST::state.isRobot_A = false;
    }

    //fastADC init is in own cpp file
}


void input_motionSense_update() {
    _input_motionSense_readSpeed();

    // IN::motionSens.motSpeed = 0.25f * (
    //     (float)speeds_ADC[0] * motion_ADC_Const +
    //     (float)speeds_ADC[1] * motion_ADC_Const +
    //     (float)speeds_ADC[2] * motion_ADC_Const +
    //     (float)speeds_ADC[3] * motion_ADC_Const );


    if(ST::state.isRobot_A) {
        speeds[0] = gpio_get_level(HW::MotionSenseCfg::A_Spd_Pin);
        speeds[1] = gpio_get_level(HW::MotionSenseCfg::B_Spd_Pin);
        speeds[2] = gpio_get_level(HW::MotionSenseCfg::C_Spd_Pin);
        speeds[3] = gpio_get_level(HW::MotionSenseCfg::D_Spd_Pin);
    } else {
        speeds[0] = !gpio_get_level(HW::MotionSenseCfg::A_Spd_Pin);
        speeds[1] = !gpio_get_level(HW::MotionSenseCfg::B_Spd_Pin);
        speeds[2] = !gpio_get_level(HW::MotionSenseCfg::C_Spd_Pin);
        speeds[3] = !gpio_get_level(HW::MotionSenseCfg::D_Spd_Pin);
    }
    
    uint8_t motFastCount = 0;

    for(uint8_t i = 0; i<4; i++) {
        if(speeds[i] > 0) { motFastCount++; }
    }
    if(motFastCount >= 3) {
        IN::motionSens.fastSpeed = true;
    } else {
        IN::motionSens.fastSpeed = false;
    }


    // if(ST::state.isRobot_A == true) { //ROBOT_A
    //     IN::motionSens.drbSpeed = speeds_ADC[4] * motion_ADC_Const;
    // } else { //ROBOT_B
    //     IN::motionSens.drbSpeed = 1.0f - (speeds_ADC[4] * motion_ADC_Const);
    // }
    IN::motionSens.drbF = gpio_get_level(HW::MotionSenseCfg::DRB_SW_F_Pin);
    // IN::motionSens.drbR = gpio_get_level(HW::MotionSenseCfg::DRB_SW_R_Pin);


    if(IN::motionSens.drbF || IN::motionSens.drbR) {
        IN::motionSens.hasBall = true;
    } else {
        IN::motionSens.hasBall = false;
    }
    // Serial.println(IN::motionSens.drbF);

    if(IN::motionSens.hasBall) {
        IN::motionSens.lastHeldBall_ms = ST::now_ms;
    } else {
        IN::motionSens.heldBallFor_ms = ST::now_ms;
    }

    #if DEBUG_LED_HASBALL
        if(debug_ball_LED_enabled != IN::motionSens.hasBall) {
            debug_ball_LED_enabled = IN::motionSens.hasBall;
            OUT::led_Debug.needsUpdate = true;
            if(IN::motionSens.hasBall == true) {
                OUT::led_Debug.color[2] = CRGB::Teal;
            } else {
                OUT::led_Debug.color[2] = CRGB::Black;
                // OUT::led_Debug.color[1] = CRGB::Black;
            }
        }
    #endif
}