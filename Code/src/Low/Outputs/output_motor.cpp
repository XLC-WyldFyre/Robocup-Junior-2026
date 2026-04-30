#include "Low/Outputs/output_motor.h"
#include "Low/config_hw.h"
#include "driver/gpio.h"
#include <Arduino.h>

static constexpr int motCH_A = 0;
static constexpr int motCH_B = 1;
static constexpr int motCH_C = 2;
static constexpr int motCH_D = 3;

static constexpr uint32_t PWM_MAX_VAL = (1u << HW::PWMCfg::PWM_RES) -1u;

static bool _motEn_lastState = !OUT::motorAcc.motEn; 


static uint32_t _output_motor_normDutyCycle(float duty) {
    duty = duty * ST::soccer.baseSpeed;
    duty = constrain(duty, 0.0f, 1.0f); //normalize from 0.0f to 1.0f
    duty = 0.10f + 0.80f * duty; //transforms from 0.1f to 0.9f  for maxon driver PWM range
    return (uint32_t)lround(duty * (float)PWM_MAX_VAL);
}


void output_motor_init_() {
    gpio_set_direction(HW::MotorCfg::EnPin, GPIO_MODE_OUTPUT);
    gpio_set_level(HW::MotorCfg::EnPin, OUT::motorAcc.motEn);

    gpio_set_direction(HW::MotorCfg::A_DirPin, GPIO_MODE_OUTPUT);
    gpio_set_direction(HW::MotorCfg::B_DirPin, GPIO_MODE_OUTPUT);
    gpio_set_direction(HW::MotorCfg::C_DirPin, GPIO_MODE_OUTPUT);
    gpio_set_direction(HW::MotorCfg::D_DirPin, GPIO_MODE_OUTPUT);

    
    ledcSetup(motCH_A, HW::PWMCfg::PWM_FREQ, HW::PWMCfg::PWM_RES);
    ledcSetup(motCH_B, HW::PWMCfg::PWM_FREQ, HW::PWMCfg::PWM_RES);
    ledcSetup(motCH_C, HW::PWMCfg::PWM_FREQ, HW::PWMCfg::PWM_RES);
    ledcSetup(motCH_D, HW::PWMCfg::PWM_FREQ, HW::PWMCfg::PWM_RES);

    ledcAttachPin(HW::MotorCfg::A_PwmPin, motCH_A);
    ledcAttachPin(HW::MotorCfg::B_PwmPin, motCH_B);
    ledcAttachPin(HW::MotorCfg::C_PwmPin, motCH_C);
    ledcAttachPin(HW::MotorCfg::D_PwmPin, motCH_D);

    ledcWrite(motCH_A, 0);
    ledcWrite(motCH_B, 0);
    ledcWrite(motCH_C, 0);
    ledcWrite(motCH_D, 0);
}


void output_motor_update() {

    // if(_motEn_lastState != OUT::motorAcc.motEn) { 
        gpio_set_level(HW::MotorCfg::EnPin, OUT::motorAcc.motEn);
        // _motEn_lastState = OUT::motorAcc.motEn;
    // }

    if(OUT::motorAcc.motEn) {
        if(OUT::motorAcc.mainOut[0] > 0) { gpio_set_level(HW::MotorCfg::A_DirPin, 0);}
        else { gpio_set_level(HW::MotorCfg::A_DirPin, 1); }
        ledcWrite(motCH_A, _output_motor_normDutyCycle(OUT::motorAcc.mainOut[0]));

        if(OUT::motorAcc.mainOut[1] > 0) { gpio_set_level(HW::MotorCfg::B_DirPin, 0);}
        else { gpio_set_level(HW::MotorCfg::B_DirPin, 1); }
        ledcWrite(motCH_B, _output_motor_normDutyCycle(OUT::motorAcc.mainOut[1]));

        if(OUT::motorAcc.mainOut[2] > 0) { gpio_set_level(HW::MotorCfg::C_DirPin, 0);}
        else { gpio_set_level(HW::MotorCfg::C_DirPin, 1); }
        ledcWrite(motCH_C, _output_motor_normDutyCycle(OUT::motorAcc.mainOut[2]));

        if(OUT::motorAcc.mainOut[3] > 0) { gpio_set_level(HW::MotorCfg::D_DirPin, 0);}
        else { gpio_set_level(HW::MotorCfg::D_DirPin, 1); }
        ledcWrite(motCH_D, _output_motor_normDutyCycle(OUT::motorAcc.mainOut[3]));
    }
    else {
        ledcWrite(motCH_A, 0);
        ledcWrite(motCH_B, 0);
        ledcWrite(motCH_C, 0);
        ledcWrite(motCH_D, 0);
    }

    OUT::motorAcc = {};
}
