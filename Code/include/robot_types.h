#pragma once
#include <cstdint>
#include "driver/gpio.h"
#include "FastLED.h"

namespace ST {
    namespace Types {
    enum class goal_t : uint8_t { Yellow = 0, Blue = 1 }; 
        enum class mode_t : int8_t {None = -1, Off = 0, Soccer = 1, Controller = 2, KickerTest = 3}; 
        enum class role_t : int8_t {Attacker = 0, Defender = 1, Test = 2}; 
        enum class pid_t : int8_t {Slow = 0, Normal = 1, Fast = 2}; 
        enum class screen_t : int8_t {Main = 0, Settings = 1};
        struct pidCfg_t {
            float P;
            float D;
            float maxLimit;
        };
    }   
}

namespace _Raw {

    struct SW_BTNs { //button state
        bool LastPos = false;
        bool StablePos = false;
        uint32_t ChangeTime = 0;

        bool Click = false;
        bool LongPress = false;
        bool IsDown = false;
        
        uint32_t PressStartTime = 0;
        bool LongSent = false;
    };

    struct HW_BTNs { //button state
        gpio_num_t pin;
        bool LastPos = false;
        bool StablePos = false;
        uint32_t ChangeTime = 0;

        bool Click = false;
        bool LongPress = false;
        bool IsDown = false;
        
        uint32_t PressStartTime = 0;
        bool LongSent = false;
    };

    struct ST_State {
        bool isRobot_A;
        ST::Types::mode_t mode;
        ST::Types::mode_t mode_previous;
        uint32_t lastActiveTime;
    };

    struct ST_PID {
        ST::Types::pidCfg_t fastPIDCfg;
        ST::Types::pidCfg_t normalPIDCfg;
        ST::Types::pidCfg_t slowPIDCfg;
        float PID_sumP;
        float PID_sumD;
        float PID_out;
        float PID_lastErr;
        uint16_t PID_dt_last_ms;
    };

    struct ST_Attacker {
      bool lineBounceFeature;
      uint32_t lineBounceFeatureTime;
    };

    struct ST_Soccer {
        ST::Types::role_t role;
        ST::Types::goal_t enemyGoal;
        float baseSpeed;
    };

    struct ST_OffMenu {
        ST::Types::screen_t screen;
        // ST::Types::menu_t state;
        int8_t cursor;
    };

    struct ST_Controller {
        
        bool autoPlay = false;
        bool previousAutoPlay = false;

    };


struct Soccer_MSG {
    uint16_t seq = 0;
    uint8_t mode = 0;
    uint8_t role = 0;

    uint8_t drbF = 0;
    uint8_t drbR = 0;
    uint16_t ball_angle_ToNField = 0;
    uint16_t ball_distRaw = 0;

    uint8_t line_seen = 0;
    uint16_t gyro_offset_ToNField = 0;
    uint8_t goalEnemy_seen = 0;
};

struct __attribute__((packed)) ControllerRx_MSG {
    uint16_t magicHead = 0;
    
    uint8_t connected = 0;
    uint32_t seq = 0;

    uint8_t right = 0;
    uint8_t down = 0;
    uint8_t up = 0;
    uint8_t left = 0;

    uint8_t square = 0;
    uint8_t cross = 0;
    uint8_t circle = 0;
    uint8_t triangle = 0;

    uint8_t l1 = 0;
    uint8_t r1 = 0;
    uint8_t l3 = 0;
    uint8_t r3 = 0;

    uint8_t share = 0;
    uint8_t options = 0;
    uint8_t psButton = 0;
    uint8_t touchpad = 0;

    uint8_t l2Pressed = 0;
    uint8_t r2Pressed = 0;

    int16_t l2 = 0;
    int16_t r2 = 0;
    int16_t lStickX = 0;
    int16_t lStickY = 0;
    int16_t rStickX = 0;
    int16_t rStickY = 0;

    uint16_t magicTail = 0;
};

// struct __attribute__((packed)) ControllerTx_MSG {
//     uint8_t ledR = 0;
//     uint8_t ledG = 0;
//     uint8_t ledB = 0;

//     uint32_t seq = 0;
//     uint32_t uptimeMs = 0;

//     uint8_t rumbleLeft = 0;
//     uint8_t rumbleRight = 0;
//     uint8_t flashOn = 0;
//     uint8_t flashOff = 0;
// };

struct _Controller_Trigger {
    SW_BTNs pressed;

    int16_t raw = 0;
    float val = 0.0f;

};

struct _Controller_Stick {
    uint16_t X = 0;
    uint16_t Y = 0;

    float angle = 0.0f;
    float mag = 0.0f;
};

struct Controller {

    uint32_t lastReadTime = 0;

    //inputs
    uint8_t connected = 0;
    uint32_t seq = 0;
    // uint32_t uptimeMs = 0;

    SW_BTNs right;
    SW_BTNs down;
    SW_BTNs up;
    SW_BTNs left;

    SW_BTNs square;
    SW_BTNs cross;
    SW_BTNs circle;
    SW_BTNs triangle;

    SW_BTNs l1;
    SW_BTNs r1;
    SW_BTNs l3;
    SW_BTNs r3;

    SW_BTNs share;
    SW_BTNs options;
    SW_BTNs psButton;
    SW_BTNs touchpad;
    
    _Controller_Trigger l2;
    _Controller_Trigger r2;

    _Controller_Stick lStick;
    _Controller_Stick rStick;
};

struct Comms {
    uint8_t SOCCER_PEER_MAC[6] = {0};
    uint8_t CONTROLLER_PEER_MAC[6] = {0};

    bool initialized = false;
    bool onlineSoccer = false;
    bool onlineController = false;

    Soccer_MSG rx_soccer_msg = {};
    Soccer_MSG tx_soccer_msg = {};

    ControllerRx_MSG rx_controller_msg = {};
    // ControllerTx_MSG tx_controller_msg = {};

    volatile bool lastSendOk = false;
    volatile bool sendDone = false;

    volatile bool newSoccerRxPacket = false;
    volatile bool newControllerRxPacket = false;

    uint32_t lastSoccerRxMsg_ms = 0;
    uint32_t lastControllerRxMsg_ms = 0;
    uint32_t lastTxMsg_ms = 0;

    uint32_t txSeq = 0;
};

    struct Display {
        bool needsUpdate = true;
    };

    struct MotorAcc {
        float mainOut[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        bool motEn = false;

    };

    struct LED {
        bool needsUpdate = true;
        uint8_t brightness = 255;
        uint32_t color = CRGB::Black;
    };
    
    struct KickerDrb
    {
        bool drbReverseDir = false;
        bool drbEn = false;
        bool kickF = false;
        bool kickR = false;
    };



    struct Generic
    {
        bool bukajModuleEn = false;
        HW_BTNs btnPlus;
        HW_BTNs btnMinus;
    };
    
    struct Goal {
        bool seen = false;
        float angle = 0.0f; //Angle from -180 to 180
        uint16_t size = 0;  //probably needs normalisartion
    };

    struct Gyro {
        float offset = 0.0f;
        float offset_ToNField = 0.0f;
        uint8_t calSys = 0; //3 is most precise
        uint8_t calGyr = 0;
        uint8_t calAcl = 0;
        uint8_t calMag = 0; 
    };

        struct Ball {
        int16_t angle = 0; 
        int16_t angle_ToNField = 0;
        uint16_t distanceRaw = 0; //higher number is means the ball is further away
        float proximity = 0.0f; //from 1-closest to 0-furthest 
        bool nearMode = false;
        bool valid = false;
    };

    struct Line {
        int16_t angle = 0; //Angle to the line (NOT THE ESCAPE ANGLE!)
        int16_t angle_ToNField = 0; // -||-
        bool seen = false;
        float depth = 0.0f;  //Dept 0.0f means the robot is not on the line
    };

    struct MotionSense {
        // float motSpeed = 0.0f;
        // float drbSpeed = 0.0f;
        bool fastSpeed = false;
        bool drbR = false;
        bool drbF = false;
    };
}