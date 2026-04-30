#include "High/Roles/role_attacker.h"
#include "High/Roles/Primitives/prim_avoid_line.h"
#include "High/Roles/Primitives/prim_chase_ball.h"
#include "High/Roles/Primitives/prim_find_goal.h"
#include "High/Services/motion.h"
#include "High/config_app.h"


namespace Role {
    void attacker() {
        if(IN::motionSens.drbF || IN::motionSens.drbR) {
            Prim::find_goal();
        }
        
        else {
            Prim::chase_ball();
        }
    }
}



//  if(IN::line.seen) {
//             Motion::addMotion(IN::line.angle + 180, Motion::weight_Hard * IN::line.depth * (1.0f + IN::motionSens.motSpeed));
//         }