#include "Low/Inputs/input_ballIR.h"
#include "Low/config_hw.h"
#include "Low/inputs.h"
#include <Arduino.h>
#include <Wire.h>
#include "High/Services/debug.h"


// filter state
static float filteredAngle = 0.0f;
static bool hasLock = false;
static uint8_t suspectCount = 0;


static inline float angleDiffDeg(float a, float b) {
    float d = fmodf(a - b + 540.0f, 360.0f) - 180.0f;
    return fabsf(d);
}

static inline float circularLerpDeg(float from, float to, float alpha) {
    float d = fmodf(to - from + 540.0f, 360.0f) - 180.0f;
    return wrap360(from + alpha * d);
}

static uint8_t median3(uint8_t a, uint8_t b, uint8_t c) {
    if (a > b) {
        uint8_t t = a;
        a = b;
        b = t;
    }
    if (b > c) {
        uint8_t t = b;
        b = c;
        c = t;
    }
    if (a > b) {
        uint8_t t = a;
        a = b;
        b = t;
    }
    return b;
}

bool readBallRaw(uint8_t raw[12], bool &nearMode, int16_t &ballAngleRaw, uint16_t &ballRawDistance) {
    // 1) RAW register
    Wire.beginTransmission(HW::BallCfg::IR_ADRR);
    Wire.write((uint8_t)HW::BallCfg::IR_RAW_REG);
    if (Wire.endTransmission() != 0) {
        return false;
    }

    int got = Wire.requestFrom((int)HW::BallCfg::IR_ADRR, HW::BallCfg::IR_RAW_COUNT);
    if (got != HW::BallCfg::IR_RAW_COUNT) {
        while (Wire.available()) Wire.read();
        return false;
    }

    for (int i = 0; i < 12; i++) {
        raw[i] = Wire.read();
    }

    nearMode = Wire.read() ? true : false;

    // 2) ANGLE + DISTANCE register
    Wire.beginTransmission(HW::BallCfg::IR_ADRR);
    Wire.write((uint8_t)HW::BallCfg::IR_AngDis_REG);
    if (Wire.endTransmission() != 0) {
        return false;
    }

    got = Wire.requestFrom((int)HW::BallCfg::IR_ADRR, 4);
    if (got != 4) {
        while (Wire.available()) Wire.read();
        return false;
    }

    ballAngleRaw = (int16_t)((Wire.read() << 8) | Wire.read());
    ballRawDistance = (uint16_t)((Wire.read() << 8) | Wire.read());

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

    // median-of-3 proti jednorazovym ustrelom
    static uint8_t hist[12][3] = {};
    static uint8_t histPos = 0;

    for (int i = 0; i < 12; i++) {
        hist[i][histPos] = raw[i];
    }
    histPos = (histPos + 1) % 3;

    uint8_t filt[12];
    for (int i = 0; i < 12; i++) {
        filt[i] = median3(hist[i][0], hist[i][1], hist[i][2]);
    }

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
        IN::ball.valid= false;
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
        float jump = angleDiffDeg(rawAngle, filteredAngle);
        float maxJump = nearMode ? HW::BallCfg::MAX_JUMP_NEAR : HW::BallCfg::MAX_JUMP_FAR;

        if (dirStrength < 0.12f) maxJump *= 0.6f;
        if (peakRatio < 0.15f) maxJump *= 0.7f;

        if (jump <= maxJump) {
        filteredAngle = circularLerpDeg(filteredAngle, rawAngle, nearMode ? HW::BallCfg::ALPHA_NEAR : HW::BallCfg::ALPHA_FAR);
        suspectCount = 0;
        } else {
        suspectCount++;
        if (suspectCount >= 3) {
            filteredAngle = circularLerpDeg(filteredAngle, rawAngle, 0.15f);
            suspectCount = 0;
        }
        }
    }

    IN::ball.angle = (int16_t)lroundf(filteredAngle);
    // ballAngle = (ballAngle + 180) % 360;
    if (IN::ball.angle >= 360) IN::ball.angle -= 360;
    if (IN::ball.angle < 0) IN::ball.angle += 360;

    IN::ball.proximity = dirStrength;//(int16_t)lroundf(dirStrength);
    IN::ball.nearMode = nearMode;
    IN::ball.valid = true;

    #ifdef DEBUG_BALL
        Serial.print("Ang: " + IN::ball.angle);
        Serial.printf(" || Prox: %0.2f\n", IN::ball.proximity);
    #endif

    // if (millis() - lastPrintMs > 100) {
    //   lastPrintMs = millis();

    //   Serial.print("mode=");
    //   Serial.print(ballNearMode ? "near" : "far");
    //   Serial.print(" raw=[");
    //   for (int i = 0; i < 12; i++) {
    //     Serial.print((int)raw[i]);
    //     if (i != 11) Serial.print(' ');
    //   }
    //   Serial.print("] angle=");
    //   Serial.print(ballAngle, 1);
    //   Serial.print(" strength=");
    //   Serial.print(ballStrength, 3);
    //   Serial.print(" valid=");
    //   Serial.println(ballValid ? "1" : "0");
    // }
}






// static int16_t ballAngleRaw = 0;

// void input_ballIR_update() {    
//     Wire.beginTransmission(HW::BallCfg::IR_ADR);
//     Wire.write((uint8_t)HW::BallCfg::IR_REG);
//     Wire.endTransmission();

//     Wire.requestFrom((int)HW::BallCfg::IR_ADR, 4);
//     ballAngleRaw = Wire.read() << 8;
//     ballAngleRaw |= Wire.read();
//     IN::ball.distRaw = Wire.read() << 8;
//     IN::ball.distRaw |= Wire.read();

//     if(IN::ball.distRaw >= HW::BallCfg::c_ballignoreDistance) {
//         IN::ball.angle = input_wrap180(IN::line.angle);
//         IN::ball.angle_ToNField = input_wrap180(IN::line.angle - IN::gyro.offset_ToNField);
//     }
//     IN::ball.strength = constrain(((float)IN::ball.distRaw - (float)HW::BallCfg::c_ballEffectiveInputFurthes) / ((float)HW::BallCfg::c_ballEffectiveInputClosest - (float)HW::BallCfg::c_ballEffectiveInputFurthes), 0.0f, 1.0f);
// }
