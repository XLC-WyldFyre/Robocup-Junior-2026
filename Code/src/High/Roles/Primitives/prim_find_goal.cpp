#include "High/Roles/Primitives/prim_find_goal.h"
#include "High/config_app.h"
#include "High/Services/motion.h"


namespace Prim {
    void find_goal() {

        if(IN::motionSens.drbF) {
            if(IN::line.seen) {
                if(IN::line.angle >= -90 && IN::line.angle <= 90) {
                    ST::attacker.lineBounceFeature = true;
                    ST::attacker.lineBounceFeatureTime = ST::now_ms;
                }
                Motion::addMotion(IN::line.angle + 180, Motion::weight_Soft);
            }

            if(IN::goalEnemy.seen) {
                if(IN::goalEnemy.angle >= -Const::Attacker::shootAngle && IN::goalEnemy.angle <= Const::Attacker::shootAngle) { 
                    OUT::kicker_Drb.kickF = true; } 
    
                Motion::rotate(IN::goalEnemy.angle, ST::pid.slowPIDCfg);
            } else {
                Motion::rotate_ToNField(0, ST::pid.slowPIDCfg); 

                if(ST::attacker.lineBounceFeature == true && IN::line.seen == false) {
                    Motion::addMotion(180, Motion::weight_Soft);
                    if(ST::now_ms - ST::attacker.lineBounceFeatureTime >= Const::Attacker::lineBounceFeatureTime_threshold) { ST::attacker.lineBounceFeature = false; }
                } else {
                    if(IN::gyro.offset >= -Const::Attacker::ballAngleNormalSpeed_threshold && IN::gyro.offset <= Const::Attacker::ballAngleNormalSpeed_threshold) { Motion::addMotion_ToNField(0, Motion::weight_Normal); }
                    else { Motion::addMotion_ToNField(0, Motion::weight_Soft); }
                }
            } 
        }
        else if(IN::motionSens.drbR) { } 

    }
}