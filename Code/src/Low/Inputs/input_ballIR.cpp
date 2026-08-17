
#include "Low/Inputs/input_ballIR.h"
#include "Low/config_hw.h"
#include "Low/inputs.h"
#include <Arduino.h>
#include <Wire.h>
#include "High/Services/debug.h"
#include "Low/Outputs/output_display.h"

#if DEBUG_LED_LINE
uint32_t debug_last_display_time = 0;
#endif

static float filteredAngle = 0.0f;
static bool hasLock = false;
static uint8_t suspectCount = 0;

static inline float angleSignedDiffDeg(float from, float to) {
    return fmodf(to - from + 540.0f, 360.0f) - 180.0f;
}

static inline float angleAbsDiffDeg(float a, float b) {
    return fabsf(angleSignedDiffDeg(a, b));
}

static inline float circularLerpDeg(float from, float to, float alpha) {
    return wrap360(from + alpha * angleSignedDiffDeg(from, to));
}

static uint8_t median3(uint8_t a, uint8_t b, uint8_t c) {
    if (a > b) { uint8_t t = a; a = b; b = t; }
    if (b > c) { uint8_t t = b; b = c; c = t; }
    if (a > b) { uint8_t t = a; a = b; b = t; }
    return b;
}

bool readBallRaw(uint8_t raw[12], bool &nearMode, int16_t &ballAngleRaw, uint16_t &ballRawDistance) {
    Wire.beginTransmission(HW::BallCfg::IR_ADRR);
    Wire.write((uint8_t)HW::BallCfg::IR_RAW_REG);
    if (Wire.endTransmission() != 0) return false;

    int got = Wire.requestFrom((int)HW::BallCfg::IR_ADRR, HW::BallCfg::IR_RAW_COUNT);
    if (got != HW::BallCfg::IR_RAW_COUNT) {
        while (Wire.available()) Wire.read();
        return false;
    }

    for (int i = 0; i < 12; i++) raw[i] = Wire.read();
    nearMode = Wire.read() ? true : false;

    Wire.beginTransmission(HW::BallCfg::IR_ADRR);
    Wire.write((uint8_t)HW::BallCfg::IR_AngDis_REG);
    if (Wire.endTransmission() != 0) return false;

    got = Wire.requestFrom((int)HW::BallCfg::IR_ADRR, 4);
    if (got != 4) {
        while (Wire.available()) Wire.read();
        return false;
    }

    ballAngleRaw = (int16_t)((Wire.read() << 8) | Wire.read());
    ballRawDistance = (uint16_t)((Wire.read() << 8) | Wire.read());
    
    #if DEBUG_BALL_RAW
    Serial.print(">Ang: ");
    Serial.println(ballAngleRaw);
    Serial.print(">Dist: ");
    Serial.println(ballRawDistance);
    // Serial.println("");
    #endif


    return true;
}

void input_ballIR_update() {
    uint8_t raw[12];
    bool nearMode = false;
    int16_t ballAngleRaw = 0;
    uint16_t ballRawDistance = 0;

    if (!readBallRaw(raw, nearMode, ballAngleRaw, ballRawDistance)) {
        IN::ball.valid = false;
        return;
    }

    #if BALL_USE_RAW_DATA
    float dirStrength = 0.0f;
    

    dirStrength = ballRawDistance / 2500.0f;
    IN::ball.distanceRaw = ballRawDistance;


    IN::ball.angle_ToNField = (int16_t)wrap360(lroundf(-ballAngleRaw + 180.0f));
    IN::ball.angle = wrap180(IN::ball.angle_ToNField);
    IN::ball.proximity = dirStrength;
    IN::ball.nearMode = nearMode;
    IN::ball.valid = true;
    IN::ball.seen = IN::ball.valid;

    if(IN::ball.proximity >= 0.9) {
        IN::ball.close = true;
        IN::ball.lastCloseTime_ms = ST::now_ms;
    } else {
        IN::ball.close = false;
        IN::ball.closeFor_ms = ST::now_ms;
    }

    if(IN::ball.proximity >= 0.85f) {
        IN::ball.midRangle = true;
        IN::ball.lastMidRangeTime_ms = ST::now_ms;
    } else {
        IN::ball.midRangle = false;
        IN::ball.MidRangeFor_ms = ST::now_ms;
    }

    #else

    static uint8_t hist[12][3] = {};
    static uint8_t histPos = 0;

    for (int i = 0; i < 12; i++) hist[i][histPos] = raw[i];
    histPos = (histPos + 1) % 3;

    uint8_t filt[12];
    for (int i = 0; i < 12; i++) filt[i] = median3(hist[i][0], hist[i][1], hist[i][2]);

    float x = 0.0f;
    float y = 0.0f;
    int sum = 0;
    int maxVal = 0;
    int secondVal = 0;

    if (nearMode) {
        for (int i = 0; i < 12; i++) {
            int v = filt[i];
            sum += v;
            if (v > maxVal) {
                secondVal = maxVal;
                maxVal = v;
            } else if (v > secondVal) {
                secondVal = v;
            }
            float a = deg2rad(HW::BallCfg::NEAR_SENSOR_ANGLE_DEG[i]);
            x += cosf(a) * v;
            y += sinf(a) * v;
        }
    } else {
        for (int i = 0; i < 6; i++) {
            int v = filt[i];
            sum += v;
            if (v > maxVal) {
                secondVal = maxVal;
                maxVal = v;
            } else if (v > secondVal) {
                secondVal = v;
            }
            float a = deg2rad(HW::BallCfg::FAR_SENSOR_ANGLE_DEG[i]);
            x += cosf(a) * v;
            y += sinf(a) * v;
        }
    }

    if (sum <= 0 || maxVal <= 0) {
        IN::ball.valid = false;
        return;
    }

    float rawAngle = wrap360(rad2deg(atan2f(-y, x)) + HW::BallCfg::FRONT_OFFSET_DEG);
    float dirStrength = sqrtf(x * x + y * y) / (float)sum;
    float peakRatio = (maxVal > 0) ? ((float)(maxVal - secondVal) / (float)maxVal) : 0.0f;

    bool enoughEnergy = nearMode ? (sum >= HW::BallCfg::MIN_SUM_NEAR) : (sum >= HW::BallCfg::MIN_SUM_FAR);
    bool enoughDirection = nearMode ? (dirStrength >= HW::BallCfg::MIN_DIR_NEAR) : (dirStrength >= HW::BallCfg::MIN_DIR_FAR);
    bool validNow = enoughEnergy && enoughDirection;

    if (!validNow) {
        IN::ball.valid = false;
        return;
    }

    if (!hasLock) {
        filteredAngle = rawAngle;
        hasLock = true;
        suspectCount = 0;
    } else {
        float jump = angleAbsDiffDeg(filteredAngle, rawAngle);
        float maxJump = nearMode ? HW::BallCfg::MAX_JUMP_NEAR : HW::BallCfg::MAX_JUMP_FAR;

        if (dirStrength < 0.12f) maxJump *= 0.60f;
        if (peakRatio < 0.15f)   maxJump *= 0.70f;

        if (jump <= maxJump) {
            float alphaBase = nearMode ? HW::BallCfg::ALPHA_NEAR : HW::BallCfg::ALPHA_FAR;
            float alphaFast = nearMode ? 0.38f : 0.24f;
            float alpha = (jump > 10.0f) ? alphaFast : alphaBase;

            filteredAngle = circularLerpDeg(filteredAngle, rawAngle, alpha);
            suspectCount = 0;
        } else {
            suspectCount++;
            if (suspectCount >= 5) {
                filteredAngle = circularLerpDeg(filteredAngle, rawAngle, 0.08f);
                suspectCount = 0;
            }
        }
    }

    IN::ball.angle_ToNField = (int16_t)lroundf(filteredAngle);
    IN::ball.angle = wrap180(IN::ball.angle_ToNField);
    IN::ball.proximity = dirStrength;
    IN::ball.nearMode = nearMode;
    IN::ball.valid = true;

    if(IN::ball.proximity == 1.0f && IN::ball.angle == 85.0f) {
        IN::ball.seen = false;
    } else {
        IN::ball.seen = true;
    }
    #endif

   
    #if DEBUG_BALL
        Serial.print(">Ang:");
        Serial.println(IN::ball.angle);
        Serial.print(">Prox:");
        Serial.println(IN::ball.proximity, 3);
    #endif

    #if DEBUG_DISPLAY_BALL
    if (ST::now_ms - debug_last_display_time >= 100) {
        if (ST::state.mode != ST::Types::mode_t::Off && ST::state.mode == ST::state.mode_previous) {
            debug_last_display_time = ST::now_ms;
            OUT::display.needsUpdate = true;
            display.setTextSize(1);
            display.setTextColor(1);
            display.setTextWrap(false);

            display.setCursor(2, 5);
            display.print(">Ball<");

            display.setCursor(2, 21);
            display.print("Angle:");
            display.setCursor(37, 21);
            display.print(IN::ball.angle);

            display.setCursor(3, 53);
            display.print("Prox:");
            display.setCursor(41, 53);
            display.print(IN::ball.proximity);

            display.setCursor(80, 53);
            display.print("R:");
            display.setCursor(95, 53);
            display.print(ballRawDistance);

        }
    }
    #endif
}