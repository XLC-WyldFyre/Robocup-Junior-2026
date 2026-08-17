#include "High/Modes/mode_ledCal.h"
#include "Low/Outputs/output_display.h"
#include "High/Services/motion.h"

static int8_t _mot_state = 0;


static const unsigned char PROGMEM image_ButtonLeftSmall_copy_1_bits[] = {0x0c,0x0c,0x3c,0x3c,0xfc,0xfc,0x3c,0x3c,0x0c,0x0c};


static void _drawScreen() {
    OUT::display.needsUpdate = true;

    display.setTextColor(1);
    display.setTextWrap(false);
    display.setTextSize(1);

    display.setCursor(4, 53);
    display.print("RotaTe to ball");
    display.setCursor(118, 53);
    if(ST::soccer.rotateToBall) {
        display.print("1");
    } else {
        display.print("0");
    }

    // display.setCursor(4, 4);
    // display.print("HasBall");
    // display.setCursor(72, 4);
    // if(IN::comms.rx_soccer_msg.drbF) {
    //     display.print("1");
    // } else {
    //     display.print("0");
    // }

    // display.setCursor(4, 16);
    // display.print("Ball_ang_N");
    // display.setCursor(72, 16);
    // display.print(IN::comms.rx_soccer_msg.ball_angle_ToNField);

    // display.setCursor(4, 41);
    // display.print("See_Line");
    // display.setCursor(72, 41);
    // if(IN::comms.rx_soccer_msg.line_seen) {
    //     display.print("1");
    // } else {
    //     display.print("0");
    // }

    // display.setCursor(4, 28);
    // display.print("Mode:");
    // display.setCursor(71, 27);
    // if(IN::comms.rx_soccer_msg.role == (int8_t)ST::Types::role_t::Attacker) {
    //     display.print("Attack");
    // } else {
    //     display.print("Defend");
    // }
    
    // display.drawBitmap(121, 15, image_ButtonLeftSmall_copy_1_bits, 6, 10, 1);
    // display.drawBitmap(121, 3, image_ButtonLeftSmall_copy_1_bits, 6, 10, 1);
    // display.drawBitmap(121, 27, image_ButtonLeftSmall_copy_1_bits, 6, 10, 1);
    // display.drawBitmap(121, 39, image_ButtonLeftSmall_copy_1_bits, 6, 10, 1);
    // display.drawBitmap(121, 51, image_ButtonLeftSmall_copy_1_bits, 6, 10, 1);
    



    // OUT::display.needsUpdate = true;
    
    // display.setTextColor(1);
    // display.setTextWrap(false);
    // display.setCursor(17, 1);
    // display.print("...LED Cal...");

    // display.setCursor(3, 17);
    // display.print("Kicker");


    // display.setTextSize(3);
    // display.setCursor(30, 36);

    // switch (_mot_state)
    // {
    // case 1:
    //     display.print("F");
    //     break;
    // case -1:
    //     display.print("R");
    //     break;
    // default:
    //     display.print("0");
    //     break;
    // }

}


namespace Mode {
    void ledCal() {
        // ST::offMenu.screen = ST::Types::screen_t::Main;

        // OUT::motorAcc.motEn = true;

        //!!!!avoid using: IN::generic.btnPlus.LongPress;

    //     if(_mot_state == 0) {
    //         if(IN::generic.btnPlus.Click) { _mot_state = -1; }
    //         if(IN::generic.btnMinus.Click) { _mot_state = 1; }
    //     } else {
    //          if(IN::generic.btnPlus.Click || IN::generic.btnMinus.Click) {
    //             _mot_state = 0;
    //          }
    //     }


    //     switch (_mot_state) {
    //     case 1:
    //         Motion::addMotion(90, 1.0f);
    //         Motion::dribler(1);
    //         break;
    //     case -1:
    //         Motion::addMotion(-90, 1.0f);
    //         Motion::dribler(1);
    //         break;
    //     default:
    //         Motion::dribler(0);
    //         break;
    //     }

        
    
        if(IN::generic.btnPlus.Click || IN::generic.btnMinus.Click) {
            ST::soccer.rotateToBall = !ST::soccer.rotateToBall;
        }
    
        _drawScreen();
    }
}