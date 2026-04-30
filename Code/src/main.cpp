#include <Arduino.h>
#include "world.h"
#include "Low/inputs.h"
#include "High/Services/logic.h"
#include "Low/outputs.h"
#include "Low/config_hw.h"

void setup()
{
    Serial.begin(HW::GenericCfg::SERIAL_SPEED);
    Serial.print("rcj26 V0 TIME:  "); Serial.print(__TIME__); Serial.print("  DATA:  "); Serial.println(__DATE__);
    ST::resetToDefaults();
    Pheripherials::inputs_init_();
    Pheripherials::outputs_init_();
    

}

void loop()
{
    ST::now_ms = millis();
    Pheripherials::inputs_update();
    Logic::determineMode();
    Logic::applyMode();

    Pheripherials::outputs_update();
}