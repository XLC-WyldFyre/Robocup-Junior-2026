#include "Low/Inputs/input_communication.h"
#include "robot_types.h"
#include "High/config_app.h"
#include "Low/config_hw.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <Arduino.h>
#include "High/Services/debug.h"

static _Raw::Soccer_MSG isr_rx_soccer_msg = {};
static _Raw::ControllerRx_MSG isr_rx_controller_msg = {};


static bool _mac_equal(const uint8_t* a, const uint8_t* b) {
    for (uint8_t i = 0; i < 6; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    IN::comms.lastSendOk = (status == ESP_NOW_SEND_SUCCESS);
    IN::comms.sendDone = true;
}


void onDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
    // if (_mac_equal(mac_addr, IN::comms.SOCCER_PEER_MAC)) {
        if (len != sizeof(_Raw::Soccer_MSG)) return;

        memcpy(&isr_rx_soccer_msg, data, sizeof(_Raw::Soccer_MSG));
        IN::comms.newSoccerRxPacket = true;
        return;
    // }

    // if (_mac_equal(mac_addr, IN::comms.CONTROLLER_PEER_MAC)) {
    //     // Serial.println("Recieved1");
    //     if (len != sizeof(_Raw::ControllerRx_MSG)) return;
    //     // Serial.println("Recieved2");
    //     memcpy(&isr_rx_controller_msg, data, sizeof(_Raw::ControllerRx_MSG));
    //     IN::comms.newControllerRxPacket = true;
    //     return;
    // }
}


static bool _add_peer(const uint8_t mac[6]) {
    if (esp_now_is_peer_exist(mac)) return true;

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, mac, 6);
    peerInfo.channel = HW::Comms::ESPNOW_CHANNEL;
    peerInfo.ifidx = WIFI_IF_STA;
    peerInfo.encrypt = false;

    return esp_now_add_peer(&peerInfo) == ESP_OK;
}

void input_comm_init_() {
    if (ST::state.isRobot_A) {
        for (uint8_t i = 0; i < 6; i++) {
            IN::comms.SOCCER_PEER_MAC[i] = HW::Comms::SOCCER_PEER_MAC_A[i];
            IN::comms.CONTROLLER_PEER_MAC[i] = HW::Comms::CONTROLLER_PEER_MAC_A[i];
        }
    } else {
        for (uint8_t i = 0; i < 6; i++) {
            IN::comms.SOCCER_PEER_MAC[i] = HW::Comms::SOCCER_PEER_MAC_B[i];
            IN::comms.CONTROLLER_PEER_MAC[i] = HW::Comms::CONTROLLER_PEER_MAC_B[i];
        }
    }

    WiFi.setSleep(false);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    esp_wifi_set_channel(HW::Comms::ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
    
    IN::comms.initialized = true;
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        IN::comms.initialized = false;
        return;
    }

    if (esp_now_register_send_cb(onDataSent) != ESP_OK) {
        Serial.println("Send callback register failed");
        IN::comms.initialized = false;
        return;
    }

    if (esp_now_register_recv_cb(onDataRecv) != ESP_OK) {
        Serial.println("Recv callback register failed");
        IN::comms.initialized = false;
        return;
    }

    if (!_add_peer(IN::comms.SOCCER_PEER_MAC)) {
        Serial.println("Failed to add soccer peer");
        IN::comms.initialized = false;
        return;
    }

    Serial.println("ESP-NOW ready with soccer peer");

    Serial.print("ESP32 STA MAC: ");
    Serial.println(WiFi.macAddress());
}


// bool input_comm_enable_controller_peer_() {
//     if (!IN::comms.initialized) return false;

//     if (!_add_peer(IN::comms.CONTROLLER_PEER_MAC)) {
//         Serial.println("Failed to add controller peer");
//         return false;
//     }

//     Serial.println("Controller peer enabled");
//     return true;
// }


static void _debug_print_comm_rx(const _Raw::Soccer_MSG& m) {
    Serial.printf(
        "seq=%d, mode:%u, role:%u | drbF:%u, drbR:%u, BAngN: %d, BDis: %d| line:%u, gyroN: %d, goal:%u\n",
        (int)m.seq,
        m.mode,
        m.role,

        m.drbF,
        m.drbR,
        (int)m.ball_angle_ToNField,
        (int)m.ball_distRaw,

        m.line_seen,
        (int)m.gyro_offset_ToNField,
        m.goalEnemy_seen
    );
}


void input_comm_update() {

    if(IN::comms.newSoccerRxPacket) {
        IN::comms.newSoccerRxPacket = false;
        memcpy(&IN::comms.rx_soccer_msg, &isr_rx_soccer_msg, sizeof(_Raw::Soccer_MSG));
        IN::comms.lastSoccerRxMsg_ms = ST::now_ms;
    }

    IN::comms.tx_soccer_msg.mode = (uint8_t)ST::state.mode;
    IN::comms.tx_soccer_msg.role = (uint8_t)ST::soccer.role;
    IN::comms.tx_soccer_msg.drbF = (uint8_t)IN::motionSens.drbF;
    IN::comms.tx_soccer_msg.drbR = (uint8_t)IN::motionSens.drbR;
    IN::comms.tx_soccer_msg.ball_angle_ToNField = (uint16_t)IN::ball.angle_ToNField;
    IN::comms.tx_soccer_msg.ball_distRaw = (uint16_t)IN::ball.distanceRaw;
    IN::comms.tx_soccer_msg.line_seen = (uint8_t)IN::line.seen;
    IN::comms.tx_soccer_msg.gyro_offset_ToNField = (uint16_t)IN::gyro.offset_ToNField;
    IN::comms.tx_soccer_msg.goalEnemy_seen = (uint8_t)IN::goalEnemy.seen;

    esp_now_send(IN::comms.SOCCER_PEER_MAC, (uint8_t*)&IN::comms.tx_soccer_msg, sizeof(IN::comms.tx_soccer_msg));

    if (ST::now_ms - IN::comms.lastSoccerRxMsg_ms <= 800){
        IN::comms.onlineSoccer == true;
        // Serial.println("online");
    } else {
        IN::comms.onlineSoccer == false;
        // Serial.println("offline");
    }

    #if DEBUG_COMMs
        _debug_print_comm_rx(IN::comms.rx_soccer_msg);
    #endif
}

// void input_comm_controller_update() {  //!!!call only when controller is connected and from controller API

//     if(IN::comms.newControllerRxPacket) {
//         IN::comms.newControllerRxPacket = false;
//         memcpy(&IN::comms.rx_controller_msg, &isr_rx_controller_msg, sizeof(_Raw::ControllerRx_MSG));
//         IN::comms.lastControllerRxMsg_ms = ST::now_ms;

//         // Serial.print(IN::comms.rx_controller_msg.l2);
//         // Serial.print("      ");
//         // Serial.println(IN::comms.rx_controller_msg.r2);
//         Serial.println(IN::comms.rx_controller_msg.seq);
//     } //else {Serial.println("no");}

//     // IN::comms.tx_controller_msg.ledR = IN::controller.ledR;
//     // IN::comms.tx_controller_msg.ledG = IN::controller.ledG;
//     // IN::comms.tx_controller_msg.ledB = IN::controller.ledB;
//     // IN::comms.tx_controller_msg.rumbleLeft = IN::controller.rumbleLeft;
//     // IN::comms.tx_controller_msg.rumbleRight = IN::controller.rumbleRight;
//     // IN::comms.tx_controller_msg.flashOn = IN::controller.flashOn;
//     // IN::comms.tx_controller_msg.flashOff = IN::controller.flashOff;

//     // esp_now_send(IN::comms.CONTROLLER_PEER_MAC, (uint8_t*)&IN::comms.tx_controller_msg, sizeof(IN::comms.tx_controller_msg));

//     if (ST::now_ms - IN::comms.lastControllerRxMsg_ms <= 100){
//         IN::comms.onlineController == true;
//         // Serial.println("online");
//     } else {
//         IN::comms.onlineController == false;
//         // Serial.println("offline");
//     }
// }
