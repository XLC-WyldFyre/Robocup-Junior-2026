#include <Arduino.h>
#include "Low/Inputs/input_ps5_controller.h"
#include "Low/config_hw.h"
#include "High/Services/debug.h"

struct BTNs {
    gpio_num_t pin;
    bool LastPos = false;
    bool StablePos = false;
    uint32_t ChangeTime = 0;
    bool Click = false;
    bool LongPress = false;
    bool IsDown = false;
    uint32_t PressStartTime = 0;
    bool LongSent = false;
};

static void _input_generic_sw_btn_update(_Raw::SW_BTNs& btn, bool raw) {
    btn.Click = false;
    btn.LongPress = false;

    if (raw != btn.StablePos) {
        if (ST::now_ms - btn.ChangeTime >= HW::GenericCfg::SW_BTN_DEBOUNCE_MS) {
            btn.StablePos = raw;
            btn.ChangeTime = ST::now_ms;
            if (btn.StablePos == true) {
                btn.PressStartTime = ST::now_ms;
                btn.LongSent = false;
            }
        }
    }

    btn.IsDown = btn.StablePos;
    btn.Click  = (btn.LastPos == true && btn.StablePos == false && btn.LongSent == false);

    if (btn.StablePos == true && btn.LongSent == false) {
        if (ST::now_ms - btn.PressStartTime >= HW::GenericCfg::SW_BTN_LONGPRESS_MS) {
            btn.LongPress = true;
            btn.LongSent  = true;
        }
    }

    btn.LastPos = btn.StablePos;
}

static void _debug_print_controller_rx(const _Raw::ControllerRx_MSG& m) {
    Serial.printf(
        "seq=%lu | dpad[R=%u D=%u U=%u L=%u] | btn[SQ=%u X=%u O=%u TRI=%u] | "
        "shoulder[L1=%u R1=%u L3=%u R3=%u] | sys[SH=%u OPT=%u PS=%u TP=%u] | "
        "trig[L2P=%u R2P=%u L2=%d R2=%d] | stick[LX=%d LY=%d RX=%d RY=%d]\n",
        (unsigned long)m.seq,
        m.right, m.down, m.up, m.left,
        m.square, m.cross, m.circle, m.triangle,
        m.l1, m.r1, m.l3, m.r3,
        m.share, m.options, m.psButton, m.touchpad,
        m.l2Pressed, m.r2Pressed,
        (int)m.l2, (int)m.r2,
        (int)m.lStickX, (int)m.lStickY,
        (int)m.rStickX, (int)m.rStickY
    );
}

// ── UART receive buffer ───────────────────────────────────────────
constexpr int UART_RX_PIN = 1;
constexpr int UART_BAUD   = 921600;
constexpr size_t MSG_SIZE = sizeof(_Raw::ControllerRx_MSG);

static uint8_t  _uart_buf[MSG_SIZE * 2]; // dvojnásobný buffer pre sync
static uint16_t _uart_buf_pos = 0;
// na začiatku súboru
static uint32_t _lastPacketTime = 0;
constexpr uint32_t CONTROLLER_TIMEOUT_MS = 500; // 500ms bez paketu = offline
// ─────────────────────────────────────────────────────────────────

void input_controller_init_() {
    // RX=1, TX=-1 (nepotrebujeme)
    Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, -1);
    Serial.println("UART receiver ready");
}

void input_controller_update() {

    IN::controller.right.Click    = false;  IN::controller.right.LongPress    = false;
    IN::controller.down.Click     = false;  IN::controller.down.LongPress     = false;
    IN::controller.up.Click       = false;  IN::controller.up.LongPress       = false;
    IN::controller.left.Click     = false;  IN::controller.left.LongPress     = false;
    IN::controller.square.Click   = false;  IN::controller.square.LongPress   = false;
    IN::controller.cross.Click    = false;  IN::controller.cross.LongPress    = false;
    IN::controller.circle.Click   = false;  IN::controller.circle.LongPress   = false;
    IN::controller.triangle.Click = false;  IN::controller.triangle.LongPress = false;
    IN::controller.l1.Click       = false;  IN::controller.l1.LongPress       = false;
    IN::controller.r1.Click       = false;  IN::controller.r1.LongPress       = false;
    IN::controller.l3.Click       = false;  IN::controller.l3.LongPress       = false;
    IN::controller.r3.Click       = false;  IN::controller.r3.LongPress       = false;
    IN::controller.share.Click    = false;  IN::controller.share.LongPress    = false;
    IN::controller.options.Click  = false;  IN::controller.options.LongPress  = false;
    IN::controller.psButton.Click = false;  IN::controller.psButton.LongPress = false;
    IN::controller.touchpad.Click = false;  IN::controller.touchpad.LongPress = false;
    IN::controller.l2.pressed.Click = false; IN::controller.l2.pressed.LongPress = false;
    IN::controller.r2.pressed.Click = false; IN::controller.r2.pressed.LongPress = false;

    // Načítaj všetky dostupné byty
    while (Serial2.available()) {
        if (_uart_buf_pos < sizeof(_uart_buf)) {
            _uart_buf[_uart_buf_pos++] = Serial2.read();
        } else {
            Serial2.read(); // buffer plný — zahoď
        }
    }

    // Spracuj VŠETKY kompletné pakety, použi len POSLEDNÝ
    _Raw::ControllerRx_MSG latest = {};
    bool gotPacket = false;

    while (_uart_buf_pos >= MSG_SIZE) {
        uint16_t head, tail;
        memcpy(&head, &_uart_buf[0], 2);
        memcpy(&tail, &_uart_buf[MSG_SIZE - 2], 2);

        if (head == 0xA55A && tail == 0x55AA) {
            // Platný — ulož ako latest ale POKRAČUJ ďalej
            memcpy(&latest, _uart_buf, MSG_SIZE);
            gotPacket = true;
            _uart_buf_pos -= MSG_SIZE;
            memmove(_uart_buf, _uart_buf + MSG_SIZE, _uart_buf_pos);
        } else {
            // Neplatný byte — zahoď a hľadaj ďalej
            _uart_buf_pos--;
            memmove(_uart_buf, _uart_buf + 1, _uart_buf_pos);
        }
    }

    if (!gotPacket) {
        // ✅ Nie hneď offline — len ak trvá dlho
        if (ST::now_ms - _lastPacketTime >= CONTROLLER_TIMEOUT_MS) {
            IN::controller.connected = false;
        }
        return;
    }

    // Platný paket prišiel — aktualizuj čas
    _lastPacketTime = ST::now_ms;
    IN::controller.connected = true;
    // ... zvyšok spracovania ostáva rovnaký

    // Teraz spracuj len ten NAJNOVŠÍ paket
    IN::comms.rx_controller_msg = latest;

    #if DEBUG_CONTROLLER
    _debug_print_controller_rx(IN::comms.rx_controller_msg);
    #endif

    IN::controller.connected = true;
    IN::controller.seq = IN::comms.rx_controller_msg.seq;

    _input_generic_sw_btn_update(IN::controller.right,    IN::comms.rx_controller_msg.right);
    _input_generic_sw_btn_update(IN::controller.down,     IN::comms.rx_controller_msg.down);
    _input_generic_sw_btn_update(IN::controller.up,       IN::comms.rx_controller_msg.up);
    _input_generic_sw_btn_update(IN::controller.left,     IN::comms.rx_controller_msg.left);
    _input_generic_sw_btn_update(IN::controller.square,   IN::comms.rx_controller_msg.square);
    _input_generic_sw_btn_update(IN::controller.cross,    IN::comms.rx_controller_msg.cross);
    _input_generic_sw_btn_update(IN::controller.circle,   IN::comms.rx_controller_msg.circle);
    _input_generic_sw_btn_update(IN::controller.triangle, IN::comms.rx_controller_msg.triangle);
    _input_generic_sw_btn_update(IN::controller.l1,       IN::comms.rx_controller_msg.l1);
    _input_generic_sw_btn_update(IN::controller.r1,       IN::comms.rx_controller_msg.r1);
    _input_generic_sw_btn_update(IN::controller.l3,       IN::comms.rx_controller_msg.l3);
    _input_generic_sw_btn_update(IN::controller.r3,       IN::comms.rx_controller_msg.r3);
    _input_generic_sw_btn_update(IN::controller.share,    IN::comms.rx_controller_msg.share);
    _input_generic_sw_btn_update(IN::controller.options,  IN::comms.rx_controller_msg.options);
    _input_generic_sw_btn_update(IN::controller.psButton, IN::comms.rx_controller_msg.psButton);
    _input_generic_sw_btn_update(IN::controller.touchpad, IN::comms.rx_controller_msg.touchpad);
    _input_generic_sw_btn_update(IN::controller.l2.pressed, IN::comms.rx_controller_msg.l2Pressed);
    _input_generic_sw_btn_update(IN::controller.r2.pressed, IN::comms.rx_controller_msg.r2Pressed);

    float l2Val = (float)IN::comms.rx_controller_msg.l2 * HW::Controller::TRIGG_CONST;
    if (l2Val < HW::Controller::DEAD_CONST) l2Val = 0.0f;
    IN::controller.l2.raw = IN::comms.rx_controller_msg.l2;
    IN::controller.l2.val = l2Val;

    float r2Val = (float)IN::comms.rx_controller_msg.r2 * HW::Controller::TRIGG_CONST;
    if (r2Val < HW::Controller::DEAD_CONST) r2Val = 0.0f;
    IN::controller.r2.raw = IN::comms.rx_controller_msg.r2;
    IN::controller.r2.val = r2Val;

    float lx = (float)IN::comms.rx_controller_msg.lStickX * HW::Controller::TRIGG_CONST;
    float ly = (float)IN::comms.rx_controller_msg.lStickY * HW::Controller::TRIGG_CONST;
    float lmag = sqrtf(lx*lx + ly*ly);
    if (lmag < HW::Controller::DEAD_CONST) { lx = ly = lmag = 0.0f; }
    float lang = atan2f(ly, lx) * 180.0f / M_PI - 90.0f;
    if (lang < 0.0f) lang += 360.0f;
    IN::controller.lStick.X     = IN::comms.rx_controller_msg.lStickX;
    IN::controller.lStick.Y     = IN::comms.rx_controller_msg.lStickY;
    IN::controller.lStick.angle = lang;
    IN::controller.lStick.mag   = lmag;

    float rx = (float)IN::comms.rx_controller_msg.rStickX * HW::Controller::TRIGG_CONST;
    float ry = (float)IN::comms.rx_controller_msg.rStickY * HW::Controller::TRIGG_CONST;
    float rmag = sqrtf(rx*rx + ry*ry);
    if (rmag < HW::Controller::DEAD_CONST) { rx = ry = rmag = 0.0f; }
    float rang = atan2f(ry, rx) * 180.0f / M_PI - 90.0f;
    if (rang < 0.0f) rang += 360.0f;
    IN::controller.rStick.X     = IN::comms.rx_controller_msg.rStickX;
    IN::controller.rStick.Y     = IN::comms.rx_controller_msg.rStickY;
    IN::controller.rStick.angle = rang;
    IN::controller.rStick.mag   = rmag;
}