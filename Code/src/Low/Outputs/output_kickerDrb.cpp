#include "Low/Outputs/output_kickerDrb.h"
#include "Low/config_hw.h"
#include "driver/gpio.h"

enum class ActiveKick {
    None,
    Front,
    Rear
};

static constexpr int KICK_TIME_DELAY = 1000;
static constexpr int KICK_TIME_MS = 100;
static uint32_t lastKickTime = 0;
static bool kicking = false;
static ActiveKick activeKick = ActiveKick::None;

bool _output_kickerDrb_kick(gpio_num_t pin) {
    if(kicking){
        if(ST::now_ms - lastKickTime >= KICK_TIME_MS) {
            gpio_set_level(pin, 0);
            kicking = false;
            lastKickTime = ST::now_ms;
            return false;
        }
    } else {
        if(ST::now_ms - lastKickTime >= KICK_TIME_DELAY) {
            gpio_set_level(pin, 1);
            kicking = true;
            lastKickTime = ST::now_ms;
            return true;
        }    
    }
}

static void _finishKick()
{
    if (activeKick == ActiveKick::Front) {
        gpio_set_level(HW::KickerDrbCfg::Kicker_F_Pin, 0);
    } else if (activeKick == ActiveKick::Rear) {
        gpio_set_level(HW::KickerDrbCfg::Kicker_R_Pin, 0);
    }

    kicking = false;
    activeKick = ActiveKick::None;
    lastKickTime = ST::now_ms;
}

static void _startKick(ActiveKick kick)
{
    if (kick == ActiveKick::Front) {
        gpio_set_level(HW::KickerDrbCfg::Kicker_F_Pin, 1);
        activeKick = ActiveKick::Front;
    } else if (kick == ActiveKick::Rear) {
        gpio_set_level(HW::KickerDrbCfg::Kicker_R_Pin, 1);
        activeKick = ActiveKick::Rear;
    }

    kicking = true;
    lastKickTime = ST::now_ms;
}


void output_kickerDrb_init_() {
    gpio_set_direction(HW::KickerDrbCfg::DRB_DirPin, GPIO_MODE_OUTPUT);
    gpio_set_level(HW::KickerDrbCfg::DRB_DirPin, OUT::kicker_Drb.drbReverseDir);

    gpio_set_direction(HW::KickerDrbCfg::Kicker_F_Pin, GPIO_MODE_OUTPUT);
    gpio_set_level(HW::KickerDrbCfg::Kicker_F_Pin, OUT::kicker_Drb.kickR);
    gpio_set_direction(HW::KickerDrbCfg::Kicker_R_Pin, GPIO_MODE_OUTPUT);
    gpio_set_level(HW::KickerDrbCfg::Kicker_R_Pin, OUT::kicker_Drb.kickR);

    gpio_set_direction(HW::KickerDrbCfg::DRB_PwmPin, GPIO_MODE_OUTPUT);
    gpio_set_level(HW::KickerDrbCfg::DRB_PwmPin, OUT::kicker_Drb.drbEn);

}


void output_kickerDrb_update() {
    gpio_set_level(HW::KickerDrbCfg::DRB_DirPin, OUT::kicker_Drb.drbReverseDir);
    gpio_set_level(HW::KickerDrbCfg::DRB_PwmPin, OUT::kicker_Drb.drbEn);

    if(kicking) {
        if(ST::now_ms - lastKickTime >= KICK_TIME_MS) {
            _finishKick();
        }
    } else {
        if(ST::now_ms - lastKickTime >= KICK_TIME_DELAY) {
            if(OUT::kicker_Drb.kickF == true) {
                _startKick(ActiveKick::Front);
            }
            else if(OUT::kicker_Drb.kickR == true) {
                _startKick(ActiveKick::Rear);
            }
        }
    }
    
    
    OUT::kicker_Drb.kickF = false;
    OUT::kicker_Drb.kickR = false;
}