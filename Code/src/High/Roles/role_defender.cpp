#include "High/Roles/role_defender.h"
#include "High/Roles/role_attacker.h"
#include "High/Roles/Primitives/prims.h"
#include "High/Services/motion.h"
#include "High/config_app.h"
#include "High/Modes/mode_soccer.h"
#include "High/Services/debug.h"
#include "Low/inputs.h"

static bool kicking = false;
static bool midkicking = false;

namespace Role {
    void defender() {

        
        if(ST::soccer.defender.changed_role) {

            if(ST::now_ms - ST::soccer.defender.changed_role_time >= 350){
                ST::soccer.defender.changed_role = false;
            }
            Motion::rotate_ToNField(0.0f, ST::pid.normalPIDCfg);
            Motion::addMotion_ToNField(180.0f, 0.4f);

        } else {


            if(ST::soccer.defender.startedOnLine) {


                Motion::dribler(1);

                #if DEFENDER_CHANGE_TO_ATTACKER
                if(ST::now_ms - IN::motionSens.heldBallFor_ms >= 100) {
                    Role::changeRole();
                    #if !ENABLE_BALL_ROLE_CHANGE_BACK
                        IN::comms.soccer_send_code = 1;
                    #endif
                } else {
                    if(ST::soccer.defender.defending) {
                        Prim::defend_goal();
                    } else {
                        // ST::soccer.defender.defending = true;
                        ST::soccer.defender.attacking = false;
                        ST::soccer.defender.startedOnLine = false;
                    }
                }
                #endif

                #if DEFENDER_KICK_TO_GOAL
                // if (ST::now_ms - IN::motionSens.heldBallFor_ms >= 100) {
                if(IN::motionSens.hasBall) {

                    if(ST::now_ms - IN::motionSens.heldBallFor_ms >= 500) {
                        Motion::kickFront();
                        kicking = false;
                        midkicking = false;
                    } else {
                        Motion::rotate_ToNField(-35.0f, ST::pid.slowPIDCfg);
                        Motion::addMotion_ToNField(-35.0f, 0.5f);
                    }

                } else if(((kicking || ST::now_ms - IN::ball.closeFor_ms >= 500) && (IN::ball.angle <= 35 && IN::ball.angle >= -35))){
                    kicking = true;
                    // midkicking = false;
                    Prim::chase_ball();
                } else if((midkicking || ST::now_ms - IN::ball.MidRangeFor_ms >= 1500)  && (IN::ball.angle <= 35 && IN::ball.angle >= -35)) {
                    midkicking = true;
                    Prim::chase_ball();
                } else {
                    Prim::defend_goal();
                    if(ST::now_ms - IN::line.lastSeen_ms >= 500 || ST::now_ms - IN::pixyRear.goalOur.lastSeen_ms >= 1000) {
                        Role::changeRole();
                        Role::changeRole();
                    }
                }

                if(ST::now_ms - IN::ball.lastCloseTime_ms >= 250){
                    kicking = false;
                }
                
                if(ST::now_ms - IN::ball.lastMidRangeTime_ms >= 250){
                    midkicking = false;
                }

                #endif

            } else {
                Motion::dribler(0);
                if(IN::line.seen) {
                    if(ST::now_ms - IN::pixyRear.goalOur.lastSeen_ms <= 500) {
                    // if(IN::pixyRear.goalOur.angle >= -(Const::Attacker::shootAngle + 180.0f) && IN::pixyRear.goalOur.angle <= Const::Attacker::shootAngle -180.0f) { 
                        ST::soccer.defender.startedOnLine = IN::line.seen;
                        // OUT::led_Debug.needsUpdate = true;
                        // OUT::led_Debug.color[1] = CRGB::Black;
                        Motion::rotate_ToNField(0.0f, ST::pid.normalPIDCfg);
                    // } else {
                    //     Prim::defender_find_our_goal();
                    // }
                } else {
                    Prim::defender_find_our_goal();
                }
                } else {
                    Prim::defender_find_our_goal();
                }
            }
        }
    }
}
