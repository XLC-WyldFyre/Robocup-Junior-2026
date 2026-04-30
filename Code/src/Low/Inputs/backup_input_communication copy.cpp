// #include "Low/Inputs/input_communication.h"
// #include "High/config_app.h"
// #include <WiFi.h>
// #include <esp_wifi.h>
// #include <esp_now.h>
// #include <Arduino.h>

// static const uint8_t ESPNOW_CHANNEL = 1;


// struct RobotStateMsg {
//     uint8_t version;
//     uint8_t robotId;
//     uint8_t mode;
//     uint8_t hasBall;

//     uint32_t seq;
//     uint32_t uptimeMs;

//     uint8_t seesBall;
//     uint8_t reserved1;
//     uint16_t reserved2;
// };



// volatile bool g_lastSendOk = false;
// volatile bool g_sendDone = false;

// RobotStateMsg g_rxState = {};
// volatile bool g_newPacket = false;
// uint32_t g_lastPeerMsgMs = 0;
// uint32_t g_txSeq = 0;


// void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
//     g_lastSendOk = (status == ESP_NOW_SEND_SUCCESS);
//     g_sendDone = true;
// }

// void onDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
//     if (len != sizeof(RobotStateMsg)) return;

//     memcpy(&g_rxState, data, sizeof(RobotStateMsg));
//     g_lastPeerMsgMs = millis();
//     g_newPacket = true;
// }



// void input_comm_init_() {

//     IN::comms.initialized = true;

//     WiFi.mode(WIFI_STA);
//     WiFi.disconnect();
//     delay(100);

//     esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

//     Serial.print("ESP32 STA MAC Address: ");
//     Serial.println(WiFi.macAddress());

//     if (esp_now_init() != ESP_OK) {
//         Serial.println("ESP-NOW init failed");
//         IN::comms.initialized = false;
//     }

//      if (esp_now_register_send_cb(onDataSent) != ESP_OK) {
//         Serial.println("Send callback register failed");
//         IN::comms.initialized = false;
//     }

//     if (esp_now_register_recv_cb(onDataRecv) != ESP_OK) {
//         Serial.println("Recv callback register failed");
//         IN::comms.initialized = false;
//     }

//     esp_now_peer_info_t peerInfo = {};
//     memcpy(peerInfo.peer_addr, Const::PEER_MAC, 6);
//     peerInfo.channel = ESPNOW_CHANNEL;
//     peerInfo.ifidx = WIFI_IF_STA;
//     peerInfo.encrypt = false;

//     if (esp_now_add_peer(&peerInfo) != ESP_OK) {
//         Serial.println("Failed to add peer");
//         IN::comms.initialized = false;
//     }

//     Serial.println("ESP-NOW ready");
// }



// bool sendState(uint8_t robotId, uint8_t mode, bool hasBall, bool seesBall) {
//     RobotStateMsg msg = {};
//     msg.version = 1;
//     msg.robotId = robotId;
//     msg.mode = mode;
//     msg.hasBall = hasBall ? 1 : 0;
//     msg.seq = g_txSeq++;
//     msg.uptimeMs = millis();
//     msg.seesBall = seesBall ? 1 : 0;

//     g_sendDone = false;

//     esp_err_t result = esp_now_send(Const::PEER_MAC, (uint8_t*)&msg, sizeof(msg));
//     if (result != ESP_OK) {
//         Serial.printf("esp_now_send error: %d\n", result);
//         return false;
//     }

//     return true;
// }


// uint32_t lastSendMs = 0;
// uint32_t g_lastRxMs = 0;

// void input_comm_update() {
//     sendState(1, 2, false, true); // robotId, mode, hasBall, seesBall

//      if (g_newPacket) {
//         g_newPacket = false;
//         g_lastRxMs = ST::now_ms;

//         Serial.printf("RX robot=%u mode=%u hasBall=%u seq=%lu\n",
//                       g_rxState.robotId,
//                       g_rxState.mode,
//                       g_rxState.hasBall,
//                       (unsigned long)g_rxState.seq);
//     }

//     if (ST::now_ms - g_lastRxMs <= 800) {
//         IN::comms.online = true;
//     } else IN::comms.online = false;
// }

















// #include "Low/Inputs/input_communication.h"
// #include "High/config_app.h"
// #include <WiFi.h>
// #include <esp_wifi.h>
// #include <esp_now.h>
// #include <Arduino.h>

// static const uint8_t ESPNOW_CHANNEL = 1;


// static const uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// struct HelloMsg {
//     uint8_t type;      // 0xA1 = hello
//     uint8_t version;
//     uint8_t reserved1;
//     uint8_t reserved2;
// };

// uint8_t g_peerMac[6] = {0};
// bool g_peerKnown = false;
// uint32_t g_lastHelloMs = 0;



// struct RobotStateMsg {
//     uint8_t version;
//     uint8_t robotId;
//     uint8_t mode;
//     uint8_t hasBall;

//     uint32_t seq;
//     uint32_t uptimeMs;

//     uint8_t seesBall;
//     uint8_t reserved1;
//     uint16_t reserved2;
// };



// volatile bool g_lastSendOk = false;
// volatile bool g_sendDone = false;

// RobotStateMsg g_rxState = {};
// volatile bool g_newPacket = false;
// uint32_t g_lastPeerMsgMs = 0;
// uint32_t g_txSeq = 0;



// void sendHello() {
//     HelloMsg msg = {};
//     msg.type = 0xA1;
//     msg.version = 1;

//     esp_now_send(BROADCAST_MAC, (uint8_t*)&msg, sizeof(msg));
// }


// void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
//     g_lastSendOk = (status == ESP_NOW_SEND_SUCCESS);
//     g_sendDone = true;
// }

// void onDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
//     if (len == sizeof(HelloMsg)) {
//         HelloMsg msg;
//         memcpy(&msg, data, sizeof(HelloMsg));

//         if (msg.type == 0xA1 && !g_peerKnown) {
//             memcpy(g_peerMac, mac_addr, 6);

//             esp_now_peer_info_t peerInfo = {};
//             memcpy(peerInfo.peer_addr, g_peerMac, 6);
//             peerInfo.channel = ESPNOW_CHANNEL;
//             peerInfo.ifidx = WIFI_IF_STA;
//             peerInfo.encrypt = false;

//             if (!esp_now_is_peer_exist(g_peerMac)) {
//                 if (esp_now_add_peer(&peerInfo) == ESP_OK) {
//                     g_peerKnown = true;
//                     Serial.print("Peer discovered: ");
//                     Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
//                                   g_peerMac[0], g_peerMac[1], g_peerMac[2],
//                                   g_peerMac[3], g_peerMac[4], g_peerMac[5]);
//                 }
//             } else {
//                 g_peerKnown = true;
//             }
//         }
//         return;
//     }

//     if (len != sizeof(RobotStateMsg)) return;

//     memcpy(&g_rxState, data, sizeof(RobotStateMsg));
//     g_lastPeerMsgMs = millis();
//     g_newPacket = true;
// }


// void input_comm_init_() {

//     IN::comms.initialized = true;

//     WiFi.mode(WIFI_STA);
//     WiFi.disconnect();
//     delay(100);

//     esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

//     Serial.print("ESP32 STA MAC Address: ");
//     Serial.println(WiFi.macAddress());

//     if (esp_now_init() != ESP_OK) {
//         Serial.println("ESP-NOW init failed");
//         IN::comms.initialized = false;
//     }

//      if (esp_now_register_send_cb(onDataSent) != ESP_OK) {
//         Serial.println("Send callback register failed");
//         IN::comms.initialized = false;
//     }

//     if (esp_now_register_recv_cb(onDataRecv) != ESP_OK) {
//         Serial.println("Recv callback register failed");
//         IN::comms.initialized = false;
//     }

//     esp_now_peer_info_t peerInfo = {};
//     memcpy(peerInfo.peer_addr, BROADCAST_MAC, 6);
//     peerInfo.channel = ESPNOW_CHANNEL;
//     peerInfo.ifidx = WIFI_IF_STA;
//     peerInfo.encrypt = false;

//     if (esp_now_add_peer(&peerInfo) != ESP_OK) {
//         Serial.println("Failed to add broadcast peer");
//         IN::comms.initialized = false;
//     }

//     uint8_t primaryChannel;
//     wifi_second_chan_t secondChannel;
//     esp_wifi_get_channel(&primaryChannel, &secondChannel);

//     Serial.print("WiFi channel: ");
//     Serial.println(primaryChannel);

//     Serial.println("ESP-NOW ready");
// }



// bool sendState(uint8_t robotId, uint8_t mode, bool hasBall, bool seesBall) {
//     if (!g_peerKnown) return false;
//     RobotStateMsg msg = {};
//     msg.version = 1;
//     msg.robotId = robotId;
//     msg.mode = mode;
//     msg.hasBall = hasBall ? 1 : 0;
//     msg.seq = g_txSeq++;
//     msg.uptimeMs = millis();
//     msg.seesBall = seesBall ? 1 : 0;

//     g_sendDone = false;

// esp_err_t result = esp_now_send(g_peerMac, (uint8_t*)&msg, sizeof(msg));
//     if (result != ESP_OK) {
//         Serial.printf("esp_now_send error: %d\n", result);
//         return false;
//     }

//     return true;
// }


// uint32_t lastSendMs = 0;
// uint32_t g_lastRxMs = 0;


// void input_comm_update() {
//     if (!g_peerKnown) {
//         if (ST::now_ms - g_lastHelloMs >= 300) {
//             g_lastHelloMs = ST::now_ms;
//             sendHello();
//         }
//         IN::comms.online = false;
//         return;
//     }

//     if (ST::now_ms - lastSendMs >= 200) {
//         lastSendMs = ST::now_ms;
//         sendState(1, 2, false, true);
//     }

//     if (g_newPacket) {
//         g_newPacket = false;
//         g_lastRxMs = ST::now_ms;

//         Serial.printf("RX robot=%u mode=%u hasBall=%u seq=%lu\n",
//                       g_rxState.robotId,
//                       g_rxState.mode,
//                       g_rxState.hasBall,
//                       (unsigned long)g_rxState.seq);
//     }

//     IN::comms.online = ((ST::now_ms - g_lastRxMs) <= 800);
// }