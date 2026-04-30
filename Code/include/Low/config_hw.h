#pragma once
#include "driver/gpio.h"


namespace HW {

    namespace Comms {
        constexpr uint8_t SOCCER_PEER_MAC_A [6] = {0xB0, 0xA7, 0x32, 0x28, 0x60, 0xFF};
        constexpr uint8_t SOCCER_PEER_MAC_B [6] = {0xB0, 0xA7, 0x32, 0x28, 0x60, 0xFF};

        constexpr uint8_t CONTROLLER_PEER_MAC_A [6] = {0xB0, 0xA7, 0x32, 0x28, 0x60, 0x00};
        constexpr uint8_t CONTROLLER_PEER_MAC_B [6] = {0xB0, 0xA7, 0x32, 0x28, 0x60, 0x00};

        constexpr int ESPNOW_CHANNEL = 1;
    }

    namespace Controller { 
        constexpr uint8_t I2C_ADDR = 0x42;

        constexpr float TRIGG_CONST = 1.0f/255;
        constexpr float DEAD_CONST = 0.1f;
    }

    namespace MotorCfg {
        //MOTORS
        constexpr gpio_num_t EnPin = GPIO_NUM_38;
        constexpr gpio_num_t A_DirPin = GPIO_NUM_42;
        constexpr gpio_num_t A_PwmPin = GPIO_NUM_41;
        constexpr gpio_num_t B_DirPin = GPIO_NUM_48;
        constexpr gpio_num_t B_PwmPin = GPIO_NUM_47;
        constexpr gpio_num_t C_DirPin = GPIO_NUM_14;
        constexpr gpio_num_t C_PwmPin = GPIO_NUM_13;
        constexpr gpio_num_t D_DirPin = GPIO_NUM_12;
        constexpr gpio_num_t D_PwmPin = GPIO_NUM_11;

    }

    namespace PWMCfg {
        constexpr int PWM_FREQ = 1000; //Hz
        constexpr int PWM_RES = 12; //bits
    }
    
    namespace KickerDrbCfg {
        //DRIBLER
        constexpr gpio_num_t DRB_DirPin = GPIO_NUM_2;
        constexpr gpio_num_t DRB_PwmPin = GPIO_NUM_10;
        //KICKER
        constexpr gpio_num_t Kicker_F_Pin = GPIO_NUM_7;
        constexpr gpio_num_t Kicker_R_Pin = GPIO_NUM_21;
    }

    namespace DisplayCfg {
        constexpr int width = 128;
        constexpr int height = 64;
        constexpr int ADRR = 0x3C;
        constexpr int rotation = 2; 
    }

    namespace MotionSenseCfg{
        //MOTORS SENSE
        
        // constexpr gpio_num_t DRBFR_Spd_Pin = GPIO_NUM_1;
        constexpr gpio_num_t A_Spd_Pin = GPIO_NUM_15;
        constexpr gpio_num_t B_Spd_Pin = GPIO_NUM_16;
        constexpr gpio_num_t C_Spd_Pin = GPIO_NUM_17;
        constexpr gpio_num_t D_Spd_Pin = GPIO_NUM_18;
        
        
        constexpr uint16_t ADC_maxVal = 4095;

        //SWITCHES
        constexpr gpio_num_t DRB_SW_F_Pin = GPIO_NUM_40;
        constexpr gpio_num_t DRB_SW_R_Pin = GPIO_NUM_39;

    }



    namespace GenericCfg {

            constexpr int HW_BTN_DEBOUNCE_MS = 20;
            constexpr int HW_BTN_LONGPRESS_MS = 500;
        
            constexpr int SW_BTN_DEBOUNCE_MS = 20;
            constexpr int SW_BTN_LONGPRESS_MS = 500;

            //BUTTONS
            constexpr gpio_num_t BTN_inc_Pin = GPIO_NUM_20;
            constexpr gpio_num_t BTN_dec_Pin = GPIO_NUM_19;
            //BUKAJ MODUL
            constexpr gpio_num_t BukajModule_Pin = GPIO_NUM_35;
            //I2C
            constexpr int I2C_SDA_Pin = 8;
            constexpr int I2C_SCL_Pin = 9;
            // constexpr int I2C_SPEED = 1000000UL;
            
            constexpr int I2C_SPEED = 400000UL;
            
            constexpr int SERIAL_SPEED = 921600;
    }

    namespace LineCfg{
        //LINE MUX
        constexpr int Line_A_Read_Pin = 4;
        constexpr int Line_B_Read_Pin = 5;
        constexpr int Line_X_Read_Pin = 6;
        constexpr int Line_A_Read_ADC_CH = 3;
        constexpr int Line_B_Read_ADC_CH = 4;
        constexpr int Line_X_Read_ADC_CH = 5;
        constexpr gpio_num_t LineSet0_Pin = GPIO_NUM_0;
        constexpr gpio_num_t LineSet1_Pin = GPIO_NUM_45;
        constexpr gpio_num_t LineSet2_Pin = GPIO_NUM_46;
        constexpr gpio_num_t LineSet3_Pin = GPIO_NUM_3;
        
        //line treshold sense
        constexpr int line_analog_treshold = 250;
        constexpr float line_depth_O_Val = 0.05f;
        //                                      45%   30%   15%    10%
        constexpr float line_depth_XArr[4] = {0.45f, 0.3f, 0.15f, 0.1f}; //percentage of robot on line

    }

    namespace LEDsCfg {
        constexpr int LEDSetO_Pin = 36;
        constexpr int LEDSet_Xdebug_Pin = 37;

        constexpr int LED_O_num = 32;
        constexpr int LED_X_num = 20;
        constexpr uint16_t LED_DEBUG_num = 4;

        constexpr float DEBUG_BRIGHTNESS = 0.01f;
    }

    namespace GyroCfg {
        constexpr int GYRO_ID = 55;
        constexpr int GYRO_ADR = 0x22;
    }


    namespace BallCfg {
        constexpr int IR_ADRR = 0x10;
        constexpr int IR_AngDis_REG = 0x00;
        constexpr int IR_RAW_REG = 0x01;
        constexpr int IR_RAW_COUNT = 13;

        static const float FRONT_OFFSET_DEG = 145.0f;

        static const float NEAR_SENSOR_ANGLE_DEG[12] = {
            0.0f, 30.0f, 60.0f, 90.0f, 120.0f, 150.0f,
            180.0f, 210.0f, 240.0f, 270.0f, 300.0f, 330.0f };

        static const float FAR_SENSOR_ANGLE_DEG[6] = {
            0.0f, 60.0f, 120.0f, 180.0f, 240.0f, 300.0f };

        // filter tuning
        const float ALPHA_NEAR = 0.40f;
        const float ALPHA_FAR = 0.18f;
        const float MAX_JUMP_NEAR = 55.0f;
        const float MAX_JUMP_FAR = 25.0f;

        const int MIN_SUM_NEAR = 20;
        const int MIN_SUM_FAR = 8;
        const float MIN_DIR_NEAR = 0.08f;
        const float MIN_DIR_FAR = 0.05f;

        constexpr int c_ballignoreDistance = 1000;
        constexpr int c_ballEffectiveInputClosest = 2500;
        constexpr int c_ballEffectiveInputFurthes = 2100;
    }

    namespace PixyCfg {
        constexpr int c_pixy_frameWidth = 316;  //camera pixel width
        constexpr int c_pixy_frameHeight = 208; //camera pixel height
        constexpr int c_pixy_objMinHeight = 108; //minimum height of object to be detected
        constexpr float c_pixy_FOV = 60.0f;

        constexpr int c_GoalSigEnemy = 1;   //Signatures from PixyMon 
        constexpr int c_GoalSigOur = 2;

    }

}

