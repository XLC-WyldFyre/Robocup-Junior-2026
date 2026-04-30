#pragma once
#include "robot_types.h"

namespace IN {
    extern _Raw::Comms comms;
    extern _Raw::Goal goalEnemy;
    extern _Raw::Goal goalOur;
    extern _Raw::Gyro gyro;
    extern _Raw::Ball ball;
    extern _Raw::Line line;
    extern _Raw::MotionSense motionSens;
    extern _Raw::Generic generic;
    extern _Raw::Controller controller;
}

namespace OUT {
    extern _Raw::LED led_O; //brightnes je globalna cize plati len pre LED_O ziadna ina
    extern _Raw::LED led_X;
    extern _Raw::LED led_Debug;
    extern _Raw::Display display;
    extern _Raw::KickerDrb kicker_Drb;
    extern _Raw::MotorAcc motorAcc; //motor output accumulator
}


namespace ST {
    extern uint32_t now_ms;
    extern _Raw::ST_State state;
    extern _Raw::ST_PID pid;
    extern _Raw::ST_Attacker attacker;
    extern _Raw::ST_Soccer soccer;
    extern _Raw::ST_OffMenu offMenu;
    extern _Raw::ST_Controller controller;

    void resetToDefaults();
}