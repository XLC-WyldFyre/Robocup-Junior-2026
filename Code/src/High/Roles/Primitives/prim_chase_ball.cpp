#include "High/Roles/Primitives/prim_chase_ball.h"
#include "High/Services/motion.h"
#include "Low/inputs.h"
#include "High/Services/debug.h"
#include "Low/Outputs/output_display.h"
#include "Arduino.h"

static bool rotateCapturing = false;


float closeOrbitAngle(float ballAngle, float proximity) {
    float a = fabs(ballAngle);

    static constexpr float FRONT_ZONE_DEG = 90.0f;
    static constexpr float FRONT_OFFSET_K = 55.0f;
    static constexpr float orbitScale     = 1.2f;

    proximity = constrain(proximity, 0.0f, 1.0f);

    // HORA - nemenena
    if (a <= FRONT_ZONE_DEG) {
        float rad = deg2rad(ballAngle);
        float offset = FRONT_OFFSET_K * sinf(rad) * (0.6f + 0.4f * proximity);
        return wrap180((ballAngle + offset) * orbitScale);
    }

    // STRED 90..135 - upraveny ako unwrap prechod cez 180 az na opacnu -/+135
    else if (a > 90.0f && a <= 135.0f) {
        float t = (a - 90.0f) / 45.0f;      // 90 -> 0, 135 -> 1
        t = constrain(t, 0.0f, 1.0f);
        t = t * t * t * (3.0f - 2.0f * t);      // smoothstep

        float rad = deg2rad(ballAngle);
        float offset = FRONT_OFFSET_K * sinf(rad) * (0.6f + 0.4f * proximity);
        float frontVal = (ballAngle + offset) * orbitScale;

        float targetUnwrapped;
        if (ballAngle > 0.0f) {
            // ideme cez +180 az na -135 => v unwrap priestore ciel 225
            targetUnwrapped = 225.0f;
        } else {
            // ideme cez -180 az na +135 => v unwrap priestore ciel -225
            targetUnwrapped = -225.0f;
        }

        float resultUnwrapped = frontVal + (targetUnwrapped - frontVal) * t;
        return wrap180(resultUnwrapped);
    }

    // DOLE - nemenena
    else if (a > 135.0f && a <= 180.0f) {
        float t = (a - 135.0f) / 45.0f;     // 135 -> 0, 180 -> 1

        float targetAbs = 135.0f - t * 45.0f;   // 135 -> 90

        float target;
        if (ballAngle > 0.0f) {
            target = -targetAbs;   // opacna strana ako lopta
        } else {
            target = targetAbs;    // opacna strana ako lopta
        }

        return wrap180(target);
    }

    else {
        return 999.0f;
    }
}

namespace Prim {
    void chase_ball() {

        // if(IN::ball.close == false) {
        //     rotateCapturing = false;
        // }
        
        // // Motion::rotate_ToNField(0.0f, ST::pid.normalPIDCfg);
        // if((ST::now_ms - IN::ball.lastCloseTime_ms <= 1000 && ST::now_ms - IN::line.lastSeen_ms <= 1000) || rotateCapturing) {
        //     rotateCapturing = true;

        //     // if(IN::ball.angle < 7 && IN::ball.angle > -7) {
        //         //     rotAng = 0.0f;
        //     float rotAng = IN::ball.angle;
        //     Motion::rotate(rotAng, ST::pid.slowPIDCfg, 0.65f);
        //     } else {
        //         Motion::rotate_ToNField(0.0f);
        //     }

            
            if(ST::soccer.rotateToBall) {
                float rotAng = IN::ball.angle;
                Motion::rotate(rotAng, ST::pid.slowPIDCfg, 0.65f);
            } else {
                Motion::rotate_ToNField(0.0f);
            }





            
            // float rotAng = constrain(wrap180(IN::ball.angle_ToNField), -150.0f, 150.0f);
            // Motion::rotate_ToNField(rotAng, ST::pid.slowPIDCfg, 0.65f);


            
            float finalAngle;


            finalAngle = closeOrbitAngle(IN::ball.angle, IN::ball.proximity);
            // Motion::addMotion(finalAngle, constrain(0.27f * (1.8f-IN::ball.proximity), 0.25f, 0.4f)); // 0.15 
            if(IN::ball.proximity >= 0.90f) {
                Motion::addMotion(finalAngle, 0.2f);
            } else {
                Motion::addMotion(finalAngle, 0.28f);

            }
        



        // } else {

        //     float rotAng = constrain(IN::ball.angle * 0.5f, -30, 30);
        //     Motion::rotate_ToNField(rotAng, ST::pid.slowPIDCfg, 1.0f);



        //     float finalAngle;
        //     if(IN::ball.seen) {
                
                


        //         if(close) {
        //             finalAngle = closeOrbitAngle(IN::ball.angle, IN::ball.proximity);
        //             if(finalAngle != 999) {
        //                 float movSrenght = constrain(1.0f * (1.5f-IN::ball.proximity) * (1.15f - cosf(IN::ball.angle * DEG_TO_RAD)) , 0.15f, 0.25f);
        //                 Motion::addMotion(finalAngle, movSrenght);
        //             }

        //         } else {
        //             finalAngle = IN::ball.angle;
        //             Motion::addMotion(finalAngle, 0.35f);
        //         }

        //     } else{
        //         finalAngle = 999;
        //     }
        // }





        #if DEBUG_FINALANGLE 
            Serial.print(">Ang: ");
            Serial.print(finalAngle);

            Serial.print(" ||  >BAng: ");
            Serial.println(IN::ball.angle);
        #endif


        #if DEBUG_DISPLAY_FINAL_ANGLE
            if(OUT::display.needsUpdate) { 

                display.setTextSize(1);
                display.setCursor(3, 37);
                display.print("F_Ang:");
                display.setCursor(41, 37);
                display.print(finalAngle);
            }
        #endif

        // Motion::addMotion(IN::ball.angle, 0.2f);



        // Motion::rotate_ToNField(0.0f, ST::pid.normalPIDCfg, 1.0f);
        // Motion::addMotion(IN::ball.angle, 0.2f);
    }
}