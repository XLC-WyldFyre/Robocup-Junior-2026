#include "Low/Inputs/input_pixy.h"
#include <Arduino.h>
#include <Pixy2I2C.h>
#include "Low/config_hw.h"
#include "High/Services/debug.h"
#include "Low/inputs.h"

static Pixy2I2C pixyFront;
static Pixy2I2C pixyRear;

#if DEBUG_LED_GOAL
static uint8_t debug_goal_prev_state_front = 255;
static uint8_t debug_goal_prev_state_rear = 255;
#endif

void input_pixy_init_()
{
    int8_t f = pixyFront.init(0x54);          // default adresa je 0x54
    int8_t r = pixyRear.init(0x55);
    Serial.println(f == 0 ? "Front Pixy init OK" : "Front Pixy init FAIL");
    Serial.println(r == 0 ? "Rear Pixy init OK" : "Rear Pixy init FAIL");
}

float _input_pixy_xToAngle(int x){

    float cx = HW::PixyCfg::c_pixy_frameWidth * 0.5f;
    float nx = (x - cx) / cx;
    return nx * (HW::PixyCfg::c_pixy_FOV * 0.5f);

}

void input_pixy_update() { //Angle from -180 to 180
    IN::pixyFront = {};

    int8_t f = pixyFront.ccc.getBlocks();
    if(f <= 0) {
        #if DEBUG_PIXY
            Serial.println("---------------------");
        #endif
        // return;
    }

    for(int i = 0; i < f; i++) {
        auto &b = pixyFront.ccc.blocks[i];

        //!!!! pixyyy - spravit to tak ze prva polovica blokov bude zlta branka a druha polovica modra branka a nech si vyberie ze kde streli ale primarne cela branka cize asi prva nech tym preloopuje

        // IN::goalEnemy.close = false;
        // IN::goalOur.close = false;

        if(b.m_y >= HW::PixyCfg::c_pixy_objMinHeight) {
            if(b.m_signature == (uint16_t)ST::soccer.enemyGoal) { //if blue SIG == 1
                IN::pixyFront.goalEnemy.seen = true;
                IN::pixyFront.goalEnemy.lastSeen_ms = ST::now_ms;
                IN::pixyFront.goalEnemy.angle = _input_pixy_xToAngle(b.m_x);
                // IN::goalEnemy.size = b.m_width * b.m_height;
                // if(b.m_height >= 30) {
                    IN::pixyFront.goalEnemy.close = true;
                // }
                    // Serial.print(b.m_height);
                    // Serial.print("  ");

            } else if(b.m_signature != (uint16_t)ST::soccer.enemyGoal) { //if yellow SIG == 0
                IN::pixyFront.goalOur.seen = true;
                IN::pixyFront.goalOur.lastSeen_ms = ST::now_ms;
                IN::pixyFront.goalOur.angle = _input_pixy_xToAngle(b.m_x);
                // IN::goalOur.size = b.m_width * b.m_height;
                if(b.m_height >= 60) {
                    IN::pixyFront.goalOur.close = true;
                }
            }

            // Serial.print(b.m_width);
            // Serial.print("  ;   ");
            // Serial.print(b.m_height);
            // Serial.print("  ;   ||");
        }
    }

    // IN::pixyRear = {};
    IN::pixyRear.goalEnemy.seen = false;
    IN::pixyRear.goalOur.seen = false;
    int8_t r = pixyRear.ccc.getBlocks();
    if(r <= 0) {
        #if DEBUG_PIXY
            Serial.println("---------------------");
        #endif
        // return;
    }

    for(int i = 0; i < r; i++) {
        auto &b = pixyRear.ccc.blocks[i];

        //!!!! pixyyy - spravit to tak ze prva polovica blokov bude zlta branka a druha polovica modra branka a nech si vyberie ze kde streli ale primarne cela branka cize asi prva nech tym preloopuje

        // IN::goalEnemy.close = false;
        // IN::goalOur.close = false;

        if(b.m_y >= HW::PixyCfg::c_pixy_objMinHeight) {
            if(b.m_signature == (uint16_t)ST::soccer.enemyGoal) { //if blue SIG == 1
                IN::pixyRear.goalEnemy.seen = true;
                IN::pixyRear.goalEnemy.lastSeen_ms = ST::now_ms;
                IN::pixyRear.goalEnemy.angle = wrap180(_input_pixy_xToAngle(b.m_x) + 180.0f);
                // IN::goalEnemy.size = b.m_width * b.m_height;
                if(b.m_height >= 50) {
                    IN::pixyRear.goalEnemy.close = true;
                }
                #if DEBUG_PIXY 
                    Serial.print(b.m_height);
                    Serial.print("  ");
                #endif

            } else if(b.m_signature != (uint16_t)ST::soccer.enemyGoal) { //if yellow SIG == 0
                IN::pixyRear.goalOur.seen = true;
                IN::pixyRear.goalOur.lastSeen_ms = ST::now_ms;
                IN::pixyRear.goalOur.angle = wrap180(_input_pixy_xToAngle(b.m_x) + 180.0f);
                // IN::goalOur.size = b.m_width * b.m_height;
                if(b.m_height >= 60) {
                    IN::pixyRear.goalOur.close = true;
                }
            }

            // Serial.print(b.m_width);
            // Serial.print("  ;   ");
            // Serial.print(b.m_height);
            // Serial.print("  ;   ||");
        }
    }

    // #if DEBUG_PIXY_FRONT 
    //     Serial.print(IN::goalEnemy.seen);
    //     Serial.print("  ");
    //     Serial.print(IN::goalEnemy.close);
    //     Serial.print("  ");
    //     Serial.print(IN::goalEnemy.angle);
    //     Serial.print("  ||  ");

    //     Serial.print(IN::goalOur.seen);
    //     Serial.print("  ");
    //     Serial.print(IN::goalOur.close);
    //     Serial.print("  ");
    //     Serial.print(IN::goalOur.angle);
    //     Serial.println("");

    //     #endif


        
    #if DEBUG_PIXY_REAR 
        Serial.print(IN::pixyRear.goalEnemy.seen);
        Serial.print("  ");
        Serial.print(IN::pixyRear.goalEnemy.close);
        Serial.print("  ");
        Serial.print(IN::pixyRear.goalEnemy.angle);
        Serial.print("  ||  ");

        Serial.print(IN::pixyRear.goalOur.seen);
        Serial.print("  ");
        Serial.print(IN::pixyRear.goalOur.close);
        Serial.print("  ");
        Serial.print(wrap180(IN::pixyRear.goalOur.angle + 180.0f));
        Serial.println("");

        #endif


#if DEBUG_LED_GOAL //Front
{
    uint8_t debug_goal_now_state = 0;

    if (IN::pixyFront.goalEnemy.seen) {
        debug_goal_now_state = 1;   // enemy
    }
    else if (IN::pixyFront.goalOur.seen) {
        debug_goal_now_state = 2;   // our
    }
    else {
        debug_goal_now_state = 0;   // off
    }

    if (debug_goal_now_state != debug_goal_prev_state_front) {
        debug_goal_prev_state_front = debug_goal_now_state;
        OUT::led_Debug.needsUpdate = true;

        if (debug_goal_now_state == 1) {
            if (ST::soccer.enemyGoal == ST::Types::goal_t::Yellow) {
                OUT::led_Debug.color[1] = CRGB::Yellow;
            } else {
                OUT::led_Debug.color[1] = CRGB::Blue;
            }
            // Serial.println("goal state -> ENEMY");
        }
        else if (debug_goal_now_state == 2) {
            if (ST::soccer.enemyGoal == ST::Types::goal_t::Yellow) {
                OUT::led_Debug.color[1] = CRGB::Blue;
            } else {
                OUT::led_Debug.color[1] = CRGB::Yellow;
            }
            // Serial.println("goal state -> OUR");
        }
        else {
            OUT::led_Debug.color[1] = CRGB::Black;
            // Serial.println("goal state -> OFF");
        }
    }
}
#endif



#if DEBUG_LED_GOAL //REAR
{
    uint8_t debug_goal_now_state = 0;

    if (IN::pixyRear.goalEnemy.seen) {
        debug_goal_now_state = 1;   // enemy
    }
    else if (IN::pixyRear.goalOur.seen) {
        debug_goal_now_state = 2;   // our
    }
    else {
        debug_goal_now_state = 0;   // off
    }

    if (debug_goal_now_state != debug_goal_prev_state_rear) {
        debug_goal_prev_state_rear = debug_goal_now_state;
        OUT::led_Debug.needsUpdate = true;

        if (debug_goal_now_state == 1) {
            if (ST::soccer.enemyGoal == ST::Types::goal_t::Yellow) {
                OUT::led_Debug.color[3] = CRGB::Yellow;
            } else {
                OUT::led_Debug.color[3] = CRGB::Blue;
            }
            // Serial.println("goal state -> ENEMY");
        }
        else if (debug_goal_now_state == 2) {
            if (ST::soccer.enemyGoal == ST::Types::goal_t::Yellow) {
                OUT::led_Debug.color[3] = CRGB::Blue;
            } else {
                OUT::led_Debug.color[3] = CRGB::Yellow;
            }
            // Serial.println("goal state -> OUR");
        }
        else {
            OUT::led_Debug.color[3] = CRGB::Black;
            // Serial.println("goal state -> OFF");
        }
    }
}
#endif


}