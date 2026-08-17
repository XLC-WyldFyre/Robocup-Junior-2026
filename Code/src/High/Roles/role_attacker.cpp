#include "High/Roles/role_attacker.h"
#include "High/Roles/Primitives/prims.h"
#include "High/Services/motion.h"
#include "High/config_app.h"

// uint32_t testTime1 = 0;
// bool test_fw = true;



namespace Role {
    void attacker() {

        if(ST::soccer.attacker.changed_role) {
            if(ST::now_ms - ST::soccer.attacker.changed_role_time >= 250){
                ST::soccer.attacker.changed_role = false;
            }
            Motion::rotate_ToNField(0.0f, ST::pid.normalPIDCfg);
            Motion::addMotion_ToNField(0.0f, 0.3f);
        } else {


            Motion::dribler(1);

            
            // IN::motionSens.hasBall = true;
            if(ST::now_ms - IN::motionSens.heldBallFor_ms >= 250) {
                Prim::find_goal();
            } else {
                Prim::chase_ball();
            }

            // Motion::rotate_ToNField(0.0f);

            if(IN::line.seen){
                Prim::avoid_line(); //must blend vectors by line depth 
            }
        }
    }
}


        // if(ST::now_ms - testTime1 >= 1000) {
        //     testTime1 = ST::now_ms;
        //     test_fw = !test_fw;
        // }
        
        // if(test_fw) {
        //     OUT::kicker_Drb.drbReverseDir = 0;
        //         Motion::dribler(1);
        //         // Motion::addMotion(45.0f , 1.0f);
        //         Motion::addMotion(0.0f, 1.0f);
        //     } else {
        //         OUT::kicker_Drb.drbReverseDir = 1;
        //         Motion::dribler(1);
        //         // Motion::addMotion(135.0f , 1.0f);
        //         Motion::addMotion(180.0f, 1.0f);
        // }

        // Motion::rotate_ToNField(95.0f, ST::pid.normalPIDCfg, 0.01f);

        // if(IN::motionSens.drbF || IN::motionSens.drbR) {
        //     Prim::find_goal();
        // }
        
        // else {
            // if(IN::line.seen) {
            //     Prim::avoid_line();
            // } else {
            //     Prim::chase_ball();
                
            // }
        // }

//     }
// }



//  if(IN::line.seen) {
//             Motion::addMotion(IN::line.angle + 180, Motion::weight_Hard * IN::line.depth * (1.0f + IN::motionSens.motSpeed));
//         }