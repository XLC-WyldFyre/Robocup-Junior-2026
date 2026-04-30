#include "Low/Inputs/input_generic.h"
#include <Wire.h>
#include <FastADC_Mux.h>
#include "Low/config_hw.h"
#include "driver/gpio.h"


void I2C_init_() {

    Wire.setBufferSize(128);
    Wire.begin(HW::GenericCfg::I2C_SDA_Pin, HW::GenericCfg::I2C_SCL_Pin);
    Wire.setClock(HW::GenericCfg::I2C_SPEED);

    // fadcInit(7, HW::LineCfg::Line_A_Read_Pin, HW::LineCfg::Line_B_Read_Pin, HW::LineCfg::Line_X_Read_Pin, HW::MotionSenseCfg::A_Spd_Pin, HW::MotionSenseCfg::B_Spd_Pin, HW::MotionSenseCfg::C_Spd_Pin, HW::MotionSenseCfg::D_Spd_Pin);

}


void input_generic_init_() {
    gpio_set_direction(HW::GenericCfg::BTN_inc_Pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(HW::GenericCfg::BTN_inc_Pin, GPIO_PULLUP_ONLY);
    gpio_set_direction(HW::GenericCfg::BTN_dec_Pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(HW::GenericCfg::BTN_dec_Pin, GPIO_PULLUP_ONLY);

    gpio_set_direction(HW::GenericCfg::BukajModule_Pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(HW::GenericCfg::BukajModule_Pin, GPIO_PULLUP_ONLY);    

    IN::generic.btnPlus.pin = HW::GenericCfg::BTN_inc_Pin;
    IN::generic.btnMinus.pin = HW::GenericCfg::BTN_dec_Pin;
}

static void _input_generic_hw_btn_update(_Raw::HW_BTNs& btn) {
    btn.Click = false;
    btn.LongPress = false;

    bool raw = !gpio_get_level(btn.pin); //active LOW

    if(raw != btn.StablePos) { //edge detection
        if(ST::now_ms - btn.ChangeTime >= HW::GenericCfg::HW_BTN_DEBOUNCE_MS) { //debounde
            btn.StablePos = raw;
            btn.ChangeTime = ST::now_ms;
            
            if(btn.StablePos == true) {
              btn.PressStartTime = ST::now_ms;
              btn.LongSent = false;
            }
        }
    }

    btn.IsDown = btn.StablePos;
    btn.Click = (btn.LastPos == true && btn.StablePos == false && btn.LongSent == false);

    if(btn.StablePos == true && btn.LongSent == false) {
        if(ST::now_ms - btn.PressStartTime >= HW::GenericCfg::HW_BTN_LONGPRESS_MS) {
            btn.LongPress = true;
            btn.LongSent = true; //dont send again
        }
    }

    btn.LastPos = btn.StablePos;
}

void input_generic_update() {

    IN::generic.bukajModuleEn = gpio_get_level(HW::GenericCfg::BukajModule_Pin);

    _input_generic_hw_btn_update(IN::generic.btnPlus);
    _input_generic_hw_btn_update(IN::generic.btnMinus);
}
