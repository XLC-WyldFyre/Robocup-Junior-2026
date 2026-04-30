#pragma once
#include "world.h"


namespace Motion {

    constexpr float weight_Soft = 0.5f;
    constexpr float weight_Normal = 1.0f;
    constexpr float weight_Hard = 2.0f;

    void addMotion(float angle, float weight = weight_Normal);
    void addMotion_ToNField(float angle, float weight = weight_Normal);
    void rotate(float angle, ST::Types::pidCfg_t& PID_type = ST::pid.normalPIDCfg);
    void rotate_ToNField(float angle, ST::Types::pidCfg_t& PID_type = ST::pid.normalPIDCfg, float weight = 1.0f);
    void addRotation(float omega);
    void kickFront();
    void kickRear();
    void dribbler(bool on_off);
}
