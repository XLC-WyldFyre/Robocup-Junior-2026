#include "High/Services/motion.h"
#include "High/Services/motion_angles.h"
#include <Arduino.h>

static int16_t _motion_wrap360(int16_t a) { //normalize from -180 to 180
    a %= 360;
    if (a < 0) a += 360;
    return a;
}

static int16_t _motion_wrap180(int16_t a) { //normalize from -180 to 180
    a %= 360; 
    if(a<0) a+=360;
    if(a>180) a-=360;
    return a;
}

namespace Motion {

    void addMotion(float angle, float weight) { //weight - pri 1.0f je pow 50%
        int16_t a = _motion_wrap360((int16_t)lround(angle));

        OUT::motorAcc.mainOut[0] += motionAngles[a][0] * weight * ST::soccer.baseSpeed;
        OUT::motorAcc.mainOut[1] += motionAngles[a][1] * weight * ST::soccer.baseSpeed;
        OUT::motorAcc.mainOut[2] += motionAngles[a][2] * weight * ST::soccer.baseSpeed;
        OUT::motorAcc.mainOut[3] += motionAngles[a][3] * weight * ST::soccer.baseSpeed;
    }

    void addMotion_ToNField(float angle, float weight) {
        int16_t a = _motion_wrap360((int16_t)roundl(angle - IN::gyro.offset_ToNField));

        OUT::motorAcc.mainOut[0] += motionAngles[a][0] * weight * ST::soccer.baseSpeed;
        OUT::motorAcc.mainOut[1] += motionAngles[a][1] * weight * ST::soccer.baseSpeed;
        OUT::motorAcc.mainOut[2] += motionAngles[a][2] * weight * ST::soccer.baseSpeed;
        OUT::motorAcc.mainOut[3] += motionAngles[a][3] * weight * ST::soccer.baseSpeed;
    }

    void rotate(float angle, ST::Types::pidCfg_t& PID_type, float weight) {
        angle = _motion_wrap180(angle);
        float dt_s = (ST::now_ms - ST::pid.PID_dt_last_ms) * 0.001f;
        if (dt_s < 0.001f) dt_s = 0.001f;

        ST::pid.PID_sumD = ((angle - ST::pid.PID_lastErr) / dt_s) * PID_type.D;
        ST::pid.PID_lastErr = angle;
        ST::pid.PID_dt_last_ms = ST::now_ms;
        
        ST::pid.PID_sumP = (angle * PID_type.P);
        ST::pid.PID_out = (ST::pid.PID_sumP + ST::pid.PID_sumD) * 0.001f *weight; //treba doladit

        if(ST::pid.PID_out > PID_type.maxLimit) ST::pid.PID_out = PID_type.maxLimit;
        if(ST::pid.PID_out < -PID_type.maxLimit) ST::pid.PID_out = -PID_type.maxLimit;

        addRotation(ST::pid.PID_out);
    }

    void rotate_ToNField(float angle, ST::Types::pidCfg_t& PID_type, float weight) {
        angle = _motion_wrap180(angle - IN::gyro.offset_ToNField);

        float dt_s = (ST::now_ms - ST::pid.PID_dt_last_ms) * 0.001f;
        if (dt_s < 0.001f) dt_s = 0.001f;
        
        ST::pid.PID_sumD = ((angle - ST::pid.PID_lastErr) / dt_s) * PID_type.D;
        ST::pid.PID_lastErr = angle;
        ST::pid.PID_dt_last_ms = ST::now_ms;
        
        ST::pid.PID_sumP = (angle * PID_type.P);
        ST::pid.PID_out = (ST::pid.PID_sumP + ST::pid.PID_sumD) * 0.001f *weight; //treba doladit

        if(ST::pid.PID_out > PID_type.maxLimit) ST::pid.PID_out = PID_type.maxLimit;
        if(ST::pid.PID_out < -PID_type.maxLimit) ST::pid.PID_out = -PID_type.maxLimit;
        
        // Serial.println(ST::pid.PID_out);
        addRotation(ST::pid.PID_out);
    }

    void addRotation(float omega) {
        OUT::motorAcc.mainOut[0] -= omega;
        OUT::motorAcc.mainOut[1] += omega;
        OUT::motorAcc.mainOut[2] -= omega;
        OUT::motorAcc.mainOut[3] += omega;
    }


    void kickFront(){
        OUT::kicker_Drb.kickF = true;
    }

    void kickRear() {
        OUT::kicker_Drb.kickR = true;
    }

    void dribler(bool on_off, bool fast){
        OUT::kicker_Drb.drbEn = on_off;
        OUT::kicker_Drb.drbFast = fast;
    }
}