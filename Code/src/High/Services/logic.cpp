#include "High/Services/logic.h"
#include "High/Modes/mode_soccer.h"
#include "High/Modes/mode_off.h"
#include "High/Modes/mode_controller.h"
#include "High/Modes/mode_kickerTest.h"
#include "Low/Inputs/input_gyro.h"
#include <Arduino.h>




namespace Logic {

    void determineMode() {

        if(ST::state.mode == ST::Types::mode_t::Soccer) {
            if(IN::generic.btnPlus.IsDown || IN::generic.btnMinus.IsDown) {
                ST::state.mode = ST::Types::mode_t::Off;
                ST::state.lastActiveTime = ST::now_ms;
            }
        }

        else if(ST::state.mode == ST::Types::mode_t::Controller) {
                if(IN::generic.btnPlus.IsDown || IN::generic.btnMinus.IsDown) {
                ST::state.mode = ST::Types::mode_t::Off;
                ST::state.lastActiveTime = ST::now_ms;
            }
        }

        else if(ST::state.mode == ST::Types::mode_t::KickerTest) {
            if(IN::generic.btnPlus.LongPress) {
                ST::state.mode = ST::Types::mode_t::Off;
                ST::state.lastActiveTime = ST::now_ms;
            }
        }




        else if(ST::state.mode == ST::Types::mode_t::Off) {
            if(ST::offMenu.screen == ST::Types::screen_t::Main) {

                if(IN::generic.btnPlus.Click && ST::now_ms - ST::state.lastActiveTime >= 1000){
                    ST::state.mode = ST::Types::mode_t::Soccer;
                }

                if(IN::generic.btnMinus.Click && ST::now_ms - ST::state.lastActiveTime >= 1000){
                    ST::state.mode = ST::Types::mode_t::Controller;
                }

                if(IN::generic.btnPlus.LongPress){
                    IN::gyro.offset = random(-180.0f, 180.0f);
                }

                if(IN::generic.btnMinus.LongPress) {
                    ST::offMenu.screen = ST::Types::screen_t::Settings;
                }
            }
            
            else if(ST::offMenu.screen == ST::Types::screen_t::Settings) {
                if(IN::generic.btnMinus.LongPress) {
                    ST::offMenu.screen = ST::Types::screen_t::Main;
                }
            }
        }
    }

    void applyMode() {
        if(ST::state.mode == ST::Types::mode_t::Soccer) Mode::soccer();
        else if(ST::state.mode == ST::Types::mode_t::Off) Mode::off();
        else if(ST::state.mode == ST::Types::mode_t::Controller) Mode::controller();
        else if(ST::state.mode == ST::Types::mode_t::KickerTest) Mode::kickerTest();
        ST::state.mode_previous = ST::state.mode;
    }
}