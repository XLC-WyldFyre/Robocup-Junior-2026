#pragma once
#include "robot_types.h"
#include "Low/Outputs/output_LED.h"

namespace IN {
    extern _Raw::Comms comms;
    extern _Raw::Pixy pixyFront;
    extern _Raw::Pixy pixyRear;
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
    extern _Raw::LED_DEBUG led_Debug;
    extern _Raw::Display display;
    extern _Raw::KickerDrb kicker_Drb;
    extern _Raw::MotorAcc motorAcc; //motor output accumulator
}


namespace ST {
    extern uint32_t now_ms;
    extern _Raw::ST_State state;
    extern _Raw::ST_PID pid;
    // extern _Raw::ST_Attacker attacker; //uz je v soccer
    extern _Raw::ST_Soccer soccer;
    extern _Raw::ST_OffMenu offMenu;
    extern _Raw::ST_Controller controller;
}

namespace ST_CFG {
    void resetToDefaults();
    void saveToMem_SoccerState();
}