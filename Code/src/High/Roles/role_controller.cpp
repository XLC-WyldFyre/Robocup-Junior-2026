#include "High/Roles/role_controller.h"
#include "High/Services/motion.h"
// #include "High/config_app.h"
#include "High/Roles/Primitives/prim_avoid_line.h"


namespace Role {
    void controller() {
    
        if(IN::line.seen) {
            Prim::avoid_line();
        } if(IN::line.depth < 0.4f) {
            Motion::addMotion(IN::controller.lStick.angle, IN::controller.lStick.mag * (0.5f + 0.5f * IN::controller.r2.val - 0.5f * IN::controller.l2.val) * constrain((1.0f - 4*IN::line.depth), 0.0f, 1.0f));
            Motion::addRotation(IN::controller.rStick.X * 0.25f);
        }



        if(IN::controller.square.Click) {
            OUT::kicker_Drb.drbEn = !OUT::kicker_Drb.drbEn;
        }
        
        if(IN::controller.triangle.Click) {
            Motion::kickFront();
        }
        if(IN::controller.cross.Click) {
            Motion::kickRear();
        }

        if(IN::controller.l1.Click) {
            // Motion::kickRear();
        }
        //vediet zmenit farku robota

        if(IN::controller.up.Click) {
            OUT::led_Debug.needsUpdate = true;
            OUT::led_Debug.color[0] = random(0x1000000);
            OUT::led_Debug.color[1] = random(0x1000000);
            OUT::led_Debug.color[2] = random(0x1000000);
            OUT::led_Debug.color[3] = random(0x1000000);
        }
        if(IN::controller.up.LongPress) {
            OUT::led_Debug.needsUpdate = true;
            OUT::led_Debug.color[0] =  CRGB::Black;
            OUT::led_Debug.color[1] =  CRGB::Black;
            OUT::led_Debug.color[2] =  CRGB::Black;
            OUT::led_Debug.color[3] =  CRGB::Black;
        }

        if(IN::controller.left.Click) {
            OUT::led_X.needsUpdate = true;
            OUT::led_X.color = random(0x1000000);
        }
        if(IN::controller.left.LongPress) {
            OUT::led_X.needsUpdate = true;
            OUT::led_X.color =  CRGB::Black;
        }

        if(IN::controller.right.Click) {
            OUT::led_O.needsUpdate = true;
            OUT::led_O.color = random(0x1000000);
        }
        if(IN::controller.right.LongPress) {
            OUT::led_O.needsUpdate = true;
            OUT::led_O.color =  CRGB::Black;
        }

        if(IN::controller.down.Click) {
            OUT::led_Debug.needsUpdate = true;
            OUT::led_X.needsUpdate = true;
            OUT::led_O.needsUpdate = true;

            OUT::led_Debug.color[0] = CRGB::Black;
            OUT::led_Debug.color[1] = CRGB::Black;
            OUT::led_Debug.color[2] = CRGB::Black;
            OUT::led_Debug.color[3] = CRGB::Black;
            OUT::led_X.color = CRGB::Black;
            OUT::led_O.color = CRGB::Black;
        }
    }
}
