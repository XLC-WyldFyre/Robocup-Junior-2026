#include "Low/outputs.h"
#include "Low/Outputs/output_LED.h"
#include "Low/Outputs/output_motor.h"
#include "Low/Outputs/output_kickerDrb.h"
#include "Low/Outputs/output_display.h"


namespace Pheripherials {
    void outputs_init_() {
        output_LED_init_();
        output_motor_init_();
        output_kickerDrb_init_();
        output_display_init_();
    }

    void outputs_update() {
        output_motor_update();
        output_kickerDrb_update();

        if(OUT::led_O.needsUpdate || OUT::led_X.needsUpdate || OUT::led_Debug.needsUpdate) { 
            output_LED_update();
        }

        if(OUT::display.needsUpdate) {
            output_display_update();
        }
    }
}