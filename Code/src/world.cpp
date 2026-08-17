#include "world.h"
#include "High/config_app.h"
#include <Preferences.h>

namespace IN {
    _Raw::Comms comms;
    _Raw::Pixy pixyFront;
    _Raw::Pixy pixyRear;
    _Raw::Goal goalEnemy;
    _Raw::Goal goalOur;
    _Raw::Gyro gyro;
    _Raw::Ball ball;
    _Raw::Line line;
    _Raw::MotionSense motionSens;
    _Raw::Generic generic;
    _Raw::Controller controller;
}

namespace OUT {
    _Raw::LED led_O; //brightnes je globalna cize plati len pre LED_O ziadna ina
    _Raw::LED led_X;
    _Raw::LED_DEBUG led_Debug;
    _Raw::Display display;
    _Raw::KickerDrb kicker_Drb;
    _Raw::MotorAcc motorAcc;
}

namespace ST {
    uint32_t now_ms;
    _Raw::ST_State state;
    _Raw::ST_PID pid;
    // _Raw::ST_Attacker attacker; //uz je v soccer
    _Raw::ST_Soccer soccer;
    _Raw::ST_OffMenu offMenu;
    _Raw::ST_Controller controller;

}

namespace ST_CFG {
    void saveToMem_SoccerState() {
        Preferences prefs;
        prefs.begin("soccer", false);
        prefs.putBool("robotA", ST::state.isRobot_A);
        prefs.putUChar("defaulRole", (uint8_t)ST::soccer.defaultRole);
        prefs.putUChar("enemyGoal", (uint8_t)ST::soccer.enemyGoal);
        prefs.putFloat("baseSpeed", ST::soccer.baseSpeed);
        prefs.end();
    }
    
    void LoadFromMem_SoccerState() {
        Preferences prefs;
        prefs.begin("soccer", true);
        ST::state.isRobot_A = prefs.getBool("robotA", ST::state.isRobot_A);
        ST::soccer.defaultRole = (ST::Types::role_t)prefs.getUChar("defaulRole", (uint8_t)ST::soccer.defaultRole);
        ST::soccer.role = (ST::Types::role_t)prefs.getUChar("defaulRole", (uint8_t)ST::soccer.defaultRole);
        ST::soccer.enemyGoal = (ST::Types::goal_t)prefs.getUChar("enemyGoal", (uint8_t)ST::soccer.enemyGoal);
        ST::soccer.baseSpeed = prefs.getFloat("baseSpeed", ST::soccer.baseSpeed);
        prefs.end();
    }
    
    void resetToDefaults() {
        ST::state = ST_Default::state;
        ST::pid = ST_Default::pid;
        ST::soccer = ST_Default::soccer;
        ST::offMenu = ST_Default::offMenu;
    
        LoadFromMem_SoccerState();
    }
}