#include "world.h"
#include "Low/inputs.h"
#include "Low/Inputs/input_generic.h"
#include "Low/Inputs/input_ballIR.h"
#include "Low/Inputs/input_gyro.h"
#include "Low/Inputs/input_line.h"
#include "Low/Inputs/input_motionSense.h"
#include "Low/Inputs/input_pixy.h"
#include "Low/Inputs/input_communication.h"

static uint32_t times[] = {0, 0, 0, 0, 0, 0, 0};
                        // 0, 1, 2, 3, 4, 5, 6

float input_wrap180(float a) { //normalize from -180 to 180
    while (a > 180.0f)  a -= 360.0f;
    while (a <= -180.0f) a += 360.0f;
    return a;
}

float wrap360(float a) {
  while (a < 0.0f) a += 360.0f;
  while (a >= 360.0f) a -= 360.0f;
  return a;
}

float deg2rad(float d) {
  return d * 0.01745329251994329577f;
}

float rad2deg(float r) {
  return r * 57.29577951308232088f;
}



namespace Pheripherials {

    void inputs_init_() {
        I2C_init_();
        input_motionSense_init_();
        input_generic_init_();
        // input_gyro_init_();
        input_line_init_();
    //     input_pixy_init_();
        input_comm_init_();
    }

    void inputs_update() {
    
        if(ST::now_ms - times[0] >= 1) { //1kHz
            // input_gyro_update();
            times[0] = ST::now_ms;
        }
        if(ST::now_ms - times[1] >= 3) { //333Hz
            input_line_update();
            input_motionSense_update();
            times[1] = ST::now_ms;
        }
        if(ST::now_ms - times[2] >= 5) { //200Hz
            // input_ballIR_update();
            times[2] = ST::now_ms;
        }

        if(ST::now_ms - times[3] >= 100) { //10Hz
            // input_pixy_update();
            times[3] = ST::now_ms;
        }

        if(ST::now_ms - times[4] >= 33) { //30Hz
            input_generic_update();
            times[4] = ST::now_ms;
        }

        if(ST::now_ms - times[5] >= 200 || ST::state.mode == ST::Types::mode_t::Controller) { //5Hz
            input_comm_update();
            times[5] = ST::now_ms;
        }

        if(ST::state.mode == ST::Types::mode_t::Off) {
            if(IN::comms.initialized == false && ST::now_ms - times[6] >= 1000) { //esp now
                input_comm_init_();
                times[6] = ST::now_ms;
            }
            
            input_generic_update();
            times[4] = ST::now_ms;
        }
    }
}