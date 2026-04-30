#include "High/Roles/Primitives/prim_chase_ball.h"
#include "High/Services/motion.h"

namespace Prim {
    void chase_ball() {
        Motion::rotate(IN::ball.angle, ST::pid.normalPIDCfg);
        Motion::addMotion(IN::ball.angle, Motion::weight_Normal);
    }
}