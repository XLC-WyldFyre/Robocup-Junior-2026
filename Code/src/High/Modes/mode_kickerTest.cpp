#include "High/Modes/mode_kickerTest.h"
#include "Low/Outputs/output_display.h"
#include "High/Services/motion.h"

static void _drawScreen() {
    OUT::display.needsUpdate = true;

    display.setTextColor(1);
    display.setTextWrap(false);
    display.setCursor(17, 1);
    display.print("...Kick Test...");

    display.setCursor(3, 17);
    display.print("Kicker");

    if(OUT::kicker_Drb.kickF) {    
        display.setTextSize(3);
        display.setCursor(6, 36);
        display.print("F");
    }

    if(OUT::kicker_Drb.kickR) { 
        display.setTextSize(3);
        display.setCursor(30, 36);
        display.print("R"); 
    }

    display.setTextSize(1);
    display.setCursor(80, 18);
    display.print("Dribbler");


    if(OUT::kicker_Drb.drbEn) {
        display.setTextSize(1);
        display.setCursor(81, 36);
        display.print("F");
        display.setCursor(107, 36);
        display.print("R");
    }

}

namespace Mode {
    void kickerTest() {

        OUT::motorAcc.motEn = true;

        //!!!!avoid using: IN::generic.btnPlus.LongPress;

        // Serial.print("kicker Test");

        if(IN::generic.btnPlus.Click) {
            Motion::kickFront();
        }

        if(IN::generic.btnMinus.Click) {
            Motion::kickRear();
        }

        if(IN::generic.btnPlus.LongPress && ST::state.mode_previous == ST::state.mode) {
            OUT::kicker_Drb.drbEn = !OUT::kicker_Drb.drbEn;
        }

        _drawScreen();



    }
}