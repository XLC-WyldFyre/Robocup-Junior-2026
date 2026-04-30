#include "Low/Outputs/output_display.h"
#include "Low/config_hw.h"
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h> //header



// static Adafruit_SSD1306 display(HW::DisplayCfg::width, HW::DisplayCfg::height, &Wire);
Adafruit_SSD1306 display(HW::DisplayCfg::width, HW::DisplayCfg::height, &Wire, -1, 400000UL, 400000UL);

void output_display_init_() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, HW::DisplayCfg::ADRR, true, false)) {
        Serial.println("SSD1306 begin failed");
    }
    display.setRotation(HW::DisplayCfg::rotation);

}

void output_display_update() {
    display.display();
    display.clearDisplay();
    OUT::display.needsUpdate = false;
}