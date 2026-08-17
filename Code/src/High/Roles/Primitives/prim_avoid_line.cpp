#include "High/Roles/Primitives/prim_avoid_line.h"
#include "High/Services/motion.h"


namespace Prim {
    void avoid_line() {

        // Serial.println("aaaaaa");
        if(IN::line.depth >= 0.3f){
            OUT::motorAcc = {};
            Motion::addMotion(IN::line.angle + 180.f, constrain(IN::line.depth*1000.0f, 0.0f, 1.5f));

        } else {
            Motion::addMotion(IN::line.angle + 180.f, constrain(IN::line.depth*4.0f, 0.0f, 0.75f));
        }
    }
}