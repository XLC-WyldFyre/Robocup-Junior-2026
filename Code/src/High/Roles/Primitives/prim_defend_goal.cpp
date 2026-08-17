#include "High/Roles/Primitives/prim_defend_goal.h"
#include "High/Services/motion.h"
#include "Arduino.h"
#include "Low/inputs.h"

// static bool behindLine = false;
static int8_t onLine = 0;  // -1 to 1; 0 mems all good :)
static uint32_t lastOutOfBounds_ms = 0;


namespace Prim {

    void defend_goal() {
        
        // Motion::rotate(IN::pixyRear.goalOur.angle + 180.0f);
        Motion::rotate_ToNField(0.0f);
        // float rotAng = constrain(IN::ball.angle, -45.0f, 45.0f);
        // Motion::rotate_ToNField(rotAng, ST::pid.slowPIDCfg);


        
        if(IN::line.defender_outOfBounds) {
            if(IN::line.defender_crossedSide == 1) {
                // Motion::addMotion_ToNField(-90.0f, 0.3f);
                onLine = 1;
                lastOutOfBounds_ms = ST::now_ms;
            } else if (IN::line.defender_crossedSide == -1) {
                // Motion::addMotion_ToNField(90.0f, 0.3f);
                onLine = -1;
                lastOutOfBounds_ms = ST::now_ms;
            }
        }

        if(onLine == 0 && ST::now_ms - IN::pixyRear.goalOur.lastSeen_ms <= 250) {
            float movStrenght = sin(DEG_TO_RAD * IN::ball.angle);
            movStrenght = constrain(movStrenght * IN::ball.proximity, -0.25f, 0.25f);
            Motion::addMotion_ToNField(90.0f, movStrenght);
            Motion::addMotion_ToNField(0.0f, 0.15f * IN::line.depth_Y);
        } else if (onLine == 1 ||  wrap180(IN::pixyRear.goalOur.angle + 180.0f) > 10.0f) {
            float movStrenght = sin(DEG_TO_RAD * IN::ball.angle);
            movStrenght = constrain(movStrenght * IN::ball.proximity, -0.25f, -0.15f);
            Motion::addMotion_ToNField(90.0f, movStrenght);
            if(ST::now_ms - lastOutOfBounds_ms >= 250) {
                onLine = 0;
            }
        } else if (onLine == -1 ||  wrap180(IN::pixyRear.goalOur.angle + 180.0f) < -10.0f) {
            float movStrenght = sin(DEG_TO_RAD * IN::ball.angle);
            movStrenght = constrain(movStrenght * IN::ball.proximity, 0.15f, 0.25f);
            Motion::addMotion_ToNField(90.0f, movStrenght);
            if(ST::now_ms - lastOutOfBounds_ms >= 250) {
                onLine = 0;
            }
        }

        // Serial.println(wrap180(IN::pixyRear.goalOur.angle + 180.0f));


        if(IN::ball.angle >= 145 && IN::ball.angle <= -145) {
            OUT::motorAcc = {};
            OUT::motorAcc.motEn = 1;
        }

        // Serial.print(IN::line.defender_outOfBounds);
        // Serial.print("  ");
        // Serial.println(IN::line.defender_crossedSide);

    }
}