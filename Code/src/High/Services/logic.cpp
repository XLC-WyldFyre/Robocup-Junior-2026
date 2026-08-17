#include "High/Services/logic.h"
#include "High/Modes/mode_soccer.h"
#include "High/Modes/mode_off.h"
#include "High/Modes/mode_controller.h"
#include "High/Modes/mode_kickerTest.h"
#include "High/Modes/mode_ledCal.h"
// #include "High/Services/motion.h"
#include "Low/Inputs/input_gyro.h"
#include <Arduino.h>
#include "High/Services/debug.h"
#include "Low/Inputs/input_communication.h"


namespace Logic {

    void determineMode() {

        if(ST::state.mode == ST::Types::mode_t::Soccer) {
            if(IN::generic.btnPlus.IsDown || IN::generic.btnMinus.IsDown) {
                ST::state.mode = ST::Types::mode_t::Off;
                ST_CFG::resetToDefaults();
                ST::state.lastActiveTime = ST::now_ms;
            }
        }

        else if(ST::state.mode == ST::Types::mode_t::Controller) {
                if(IN::generic.btnPlus.IsDown || IN::generic.btnMinus.IsDown) {
                ST::state.mode = ST::Types::mode_t::Off;
                ST_CFG::resetToDefaults();
                ST::state.lastActiveTime = ST::now_ms;
            }
        }

        else if (ST::state.mode == ST::Types::mode_t::LedCal) {
            if(IN::generic.btnMinus.LongPress) {
                ST::state.mode = ST::Types::mode_t::Off;
                ST::state.lastActiveTime = ST::now_ms;
            }
            //logic is written in its own file 
        }


        else if(ST::state.mode == ST::Types::mode_t::KickerTest) {
            if(IN::generic.btnMinus.LongPress) {
                ST::state.mode = ST::Types::mode_t::Off;
                ST::state.lastActiveTime = ST::now_ms;
            }
            //logic is written in its own file 
        }


        else if(ST::state.mode == ST::Types::mode_t::Off) {
            if(ST::offMenu.screen == ST::Types::screen_t::Main) {

                if(IN::generic.btnPlus.IsDown && IN::generic.btnMinus.IsDown) {
                    // IN::generic.btnPlus = {};
                    // IN::generic.btnMinus = {};
                    
                    ST::state.lastActiveTime = ST::now_ms;
                    
                    IN::comms.soccer_send_code = 1;
                    // Serial.println("dasidasdbaububdusbasudbsu");
                }

                if(IN::generic.btnPlus.Click && ST::now_ms - ST::state.lastActiveTime >= 1000){
                    ST::state.mode = ST::Types::mode_t::Soccer;
                }

                #if ENABLE_CONTROLLER 
                    if(IN::generic.btnMinus.Click && ST::now_ms - ST::state.lastActiveTime >= 1000){
                        ST::state.mode = ST::Types::mode_t::Controller;
                    }
                #else
                    // :)))
                #endif
                
                if(IN::generic.btnPlus.LongPress){
                    // IN::gyro.offset = random(-180.0f, 180.0f); 
                    input_gyro_calibrate();
                }

                if(IN::generic.btnMinus.LongPress) {
                    ST::offMenu.screen = ST::Types::screen_t::Settings;
                }
            }



            
            else if(ST::offMenu.screen == ST::Types::screen_t::Settings) {
                if(IN::generic.btnMinus.LongPress) {
                    ST::offMenu.screen = ST::Types::screen_t::Main;
                }

                if(IN::generic.btnPlus.Click) { 
                    ST::offMenu.cursor--;
                    if(ST::offMenu.cursor < 0) { ST::offMenu.cursor = 4; }
                }
                if(IN::generic.btnMinus.Click) {
                    ST::offMenu.cursor++;
                    if(ST::offMenu.cursor > 4) { ST::offMenu.cursor = 0; }  
                }

                if(IN::generic.btnPlus.LongPress) {
                    switch(ST::offMenu.cursor) {
                        case 0: if(ST::soccer.enemyGoal == ST::Types::goal_t::Yellow) { ST::soccer.enemyGoal = ST::Types::goal_t::Blue; }
                                else { ST::soccer.enemyGoal = ST::Types::goal_t::Yellow; }
                                ST_CFG::saveToMem_SoccerState();
                            break;
                        case 1: ST::state.isRobot_A = !ST::state.isRobot_A;
                                if(ST::soccer.defaultRole == ST::Types::role_t::Attacker) {
                                    ST::soccer.defaultRole = ST::Types::role_t::Defender; 
                                        ST::soccer.role = ST::Types::role_t::Defender;
                                    } else {
                                        ST::soccer.defaultRole = ST::Types::role_t::Attacker;
                                        ST::soccer.role = ST::Types::role_t::Attacker;
                                    }
                                input_comm_init_();
                                ST_CFG::saveToMem_SoccerState();
                            break;
                        case 2: if(ST::soccer.defaultRole == ST::Types::role_t::Attacker) {
                                    ST::soccer.defaultRole = ST::Types::role_t::Defender; 
                                        ST::soccer.role = ST::Types::role_t::Defender;
                                    } else {
                                        ST::soccer.defaultRole = ST::Types::role_t::Attacker;
                                        ST::soccer.role = ST::Types::role_t::Attacker;
                                    }
                                ST_CFG::saveToMem_SoccerState(); 
                            break;
                        case 3: ST::state.mode = ST::Types::mode_t::LedCal;
                            break;
                        case 4: ST::state.mode = ST::Types::mode_t::KickerTest;
                            break;
                        default:
                            break;
                    }
                }

            }
        }
    }

    void applyMode() {
        if(ST::state.mode == ST::Types::mode_t::Soccer) Mode::soccer();
        else if(ST::state.mode == ST::Types::mode_t::Off) Mode::off();
        else if(ST::state.mode == ST::Types::mode_t::Controller) Mode::controller();
        else if(ST::state.mode == ST::Types::mode_t::LedCal) Mode::ledCal();
        else if(ST::state.mode == ST::Types::mode_t::KickerTest) Mode::kickerTest();
        ST::state.mode_previous = ST::state.mode;
    }
}