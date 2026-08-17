#include "High/Roles/Primitives/prim_find_goal.h"
#include "High/config_app.h"
#include "High/Services/motion.h"
#include "High/Services/debug.h"
#include "Low/inputs.h"

// #if DEBUG_LED_GOAL
//     bool debug_goal_LED_enabled = false; 
// #endif

namespace Prim {

    void find_goal() {


            if(IN::pixyFront.goalEnemy.seen) {
                if(IN::pixyFront.goalEnemy.angle >= -Const::Attacker::shootAngle && IN::pixyFront.goalEnemy.angle <= Const::Attacker::shootAngle) { 
                    if(ST::now_ms - IN::motionSens.heldBallFor_ms >= 500 || IN::line.seen) {
                        if(IN::pixyFront.goalEnemy.close || IN::line.seen) {
                            OUT::kicker_Drb.kickF = true; 
                        }
                    }
                    Motion::addMotion(IN::pixyFront.goalEnemy.angle, 0.5f);
                    Motion::rotate(IN::pixyFront.goalEnemy.angle, ST::pid.slowPIDCfg);
                } else {
                    Motion::addMotion(IN::pixyFront.goalEnemy.angle, 0.35f);
                    Motion::rotate(IN::pixyFront.goalEnemy.angle, ST::pid.slowPIDCfg);
                }
            } else if(ST::soccer.attacker.lineBounceFeature && ST::now_ms - ST::soccer.attacker.lineBounceFeatureTime <= 2000){
                Motion::addMotion_ToNField(180.0f, 0.2f);
                Motion::rotate_ToNField(0.0f, ST::pid.slowPIDCfg); 

            } else {
                if(IN::line.seen && (IN::line.angle > -90.0f && IN::line.angle < 90.0f)) {
                    ST::soccer.attacker.lineBounceFeature = true;
                    ST::soccer.attacker.lineBounceFeatureTime = ST::now_ms;
                } else {
                    if(IN::gyro.offset >= -30.0f && IN::gyro.offset <= 30.0f) {
                        Motion::addMotion_ToNField(0.0f, 0.2f);
                    }
                    Motion::rotate_ToNField(0.0f, ST::pid.slowPIDCfg); 
                }
            }
            

    }



        void defender_find_our_goal() {


            if(IN::pixyRear.goalOur.seen) {
                Motion::addMotion(IN::pixyRear.goalOur.angle, 0.35f);
                Motion::rotate(wrap180(IN::pixyRear.goalOur.angle + 180.0f), ST::pid.normalPIDCfg);
            } else if(ST::soccer.defender.lineBounceFeature && ST::now_ms - ST::soccer.defender.lineBounceFeatureTime <= 2000){
                Motion::addMotion_ToNField(0.0f, 0.35f);
                Motion::rotate_ToNField(0.0f, ST::pid.normalPIDCfg); 

            } else {
                // if(IN::line.seen && (IN::line.angle > -90.0f && IN::line.angle < 90.0f)) {
                if(IN::line.seen){
                    ST::soccer.defender.lineBounceFeature = true;
                    ST::soccer.defender.lineBounceFeatureTime = ST::now_ms;
                } else {
                    Motion::addMotion_ToNField(180.0f, 0.35f);
                    Motion::rotate_ToNField(0.0f, ST::pid.normalPIDCfg); 
                }
            }
            




        // #if DEBUG_LED_GOAL
        //     if(debug_goal_LED_enabled != IN::pixyFront.goalEnemy..seen) {
        //         debug_goal_LED_enabled = IN::pixyFront.goalEnemy..seen;
        //         OUT::led_Debug.needsUpdate = true;
        //         if(IN::pixyFront.goalEnemy..seen == true) {
        //             OUT::led_Debug.color[1] = CRGB::DarkMagenta;
        //     } 
        //     } else  if(debug_goal_LED_enabled != ST::soccer.defender.lineBounceFeature) {
        //         debug_goal_LED_enabled = ST::soccer.defender.lineBounceFeature;
        //         OUT::led_Debug.needsUpdate = true;
        //         if(ST::soccer.defender.lineBounceFeature == true) {
        //             OUT::led_Debug.color[1] = CRGB::White;
        //         }
        //     } else if(debug_goal_LED_enabled != IN::pixyFront.goalOur.seen) {
        //         debug_goal_LED_enabled = IN::pixyFront.goalOur.seen;
        //         OUT::led_Debug.needsUpdate = true;
        //         if(IN::pixyFront.goalOur.seen == true) {
        //             OUT::led_Debug.color[1] = CRGB::DarkGreen;
        //         }
        //     } else {
        //             OUT::led_Debug.color[1] = CRGB::Black;
        //         }
        // #endif
            

    }



}