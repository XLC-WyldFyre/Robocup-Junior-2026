#include "Low/Inputs/input_gyro.h"
#include "Adafruit_BNO055.h"
#include "Low/config_hw.h"

static Adafruit_BNO055 gyro = Adafruit_BNO055(HW::GyroCfg::GYRO_ID, HW::GyroCfg::GYRO_ADR, &Wire);
static float baseOffset = 0.0f;

void input_gyro_init_() {
    !gyro.begin();
    gyro.setExtCrystalUse(true);
}

static void _input_gyro_normalize() {
        if(IN::gyro.offset > 180) IN::gyro.offset -= 360;
        if(IN::gyro.offset < -180) IN::gyro.offset += 360;

        IN::gyro.offset_ToNField = IN::gyro.offset += 360;
        if(IN::gyro.offset_ToNField >= 360) IN::gyro.offset_ToNField -= 360;
    }



void input_gyro_calibrate() {
    baseOffset = IN::gyro.offset + baseOffset;

    _input_gyro_normalize();
}


void input_gyro_update(){
    sensors_event_t event;
    gyro.getEvent(&event);
    IN::gyro.offset = (float)event.orientation.x - baseOffset;
    
    _input_gyro_normalize();

    //Get callibration status
    if(ST::state.mode == ST::Types::mode_t::Off) {
        gyro.getCalibration(&IN::gyro.calSys, &IN::gyro.calGyr, &IN::gyro.calAcl, &IN::gyro.calMag);
    }
}