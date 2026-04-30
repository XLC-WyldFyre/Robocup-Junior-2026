#pragma once
#include "robot_types.h"
#include "Low/config_hw.h"

#define STOP_MOTORS false
#define DISPLAY_DEBUG false

namespace Const {

    namespace Attacker{
        constexpr uint32_t lineBounceFeatureTime_threshold = 2000;
        constexpr float shootAngle = 15.0f;
        constexpr float ballAngleNormalSpeed_threshold = 20.0f;
        constexpr float ballDecelerationConst = 0.5f;
    }
}


namespace ST_Default {

    inline const _Raw::ST_State state = {
        .isRobot_A = true,
        .mode = ST::Types::mode_t::Off,
        .mode_previous = ST::Types::mode_t::None,
        .lastActiveTime = 0
    };

    inline const _Raw::ST_PID pid = {
        .fastPIDCfg = {
            .P = 1.0f,
            .D = 1.0f,
            .maxLimit = 100.0f,
        },
        .normalPIDCfg = {
            .P = 1.0f,
            .D = 1.0f,
            .maxLimit = 10.0f,
        },
        .slowPIDCfg = {
            .P = 1.0f,
            .D = 1.0f,
            .maxLimit = 1.0f,
        },
        .PID_sumP = 0.0f,
        .PID_sumD = 0.0f,
        .PID_out = 0.0f,
        .PID_lastErr = 0.0f,
        .PID_dt_last_ms = 0,
    };

    inline const _Raw::ST_Soccer soccer = {
        .role = ST::Types::role_t::Attacker,
        .enemyGoal = ST::Types::goal_t::Yellow,
        .baseSpeed = 1.0f,
    };

    inline const _Raw::ST_OffMenu offMenu = {
        .screen = ST::Types::screen_t::Main,
        // .state = ST::Types::menu_t::None,
        .cursor = 0
    };
}

