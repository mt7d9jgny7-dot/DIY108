#include "hid.h"
#include "matrix.h"
#include "keymap.h"
#include "keycodes.h"
#include "macro.h"
#include "report.h"
#include "webhid.h"

void hid_task(void) {
    // 1. 处理 WebHID 后台延迟保存
    webhid_task();

    uint8_t kb_report[8] = {0}; 
    uint8_t nkro_bitmap[31] = {0}; 
    uint16_t cc_report = 0;
    uint8_t key_count = 0;
    int8_t mouse_dx = 0, mouse_dy = 0;
    uint8_t mouse_btn = 0;
    int8_t mouse_wheel = 0;
    static bool macro_triggered = false;

    // 2. 扫描矩阵组装报文
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            if (matrix_state[r][c]) {
                uint16_t hid_code = keymap[current_layer][r][c];
                
                if (hid_code >= MACRO_1 && hid_code <= MACRO_10) {
                    if (!macro_triggered) {
                        play_macro(hid_code - MACRO_1);
                        macro_triggered = true;
                    }
                } else if (hid_code >= 0x200) {
                    switch (hid_code) {
                        case CC_MUTE: cc_report = 0xE2; break; case CC_VOLU: cc_report = 0xE9; break;
                        case CC_VOLD: cc_report = 0xEA; break; case CC_MPLY: cc_report = 0xCD; break;
                        case CC_MNXT: cc_report = 0xB5; break; case CC_MPRV: cc_report = 0xB6; break;
                        case CC_STOP: cc_report = 0xB7; break; case CC_CALC: cc_report = 0x192; break;
                        case CC_MAIL: cc_report = 0x18A; break; case CC_MYCM: cc_report = 0x194; break;
                        case CC_WSCH: cc_report = 0x221; break; case CC_WHOM: cc_report = 0x223; break;
                        case CC_WBAK: cc_report = 0x224; break; case CC_WFWD: cc_report = 0x225; break;
                        case CC_BRIU: cc_report = 0x6F; break; case CC_BRID: cc_report = 0x70; break;
                        case CC_PWR: cc_report = 0x30; break; case CC_SLEP: cc_report = 0x32; break;
                    }
                } else if (hid_code >= 0x100) {
                    switch (hid_code) {
                        case MS_UP: mouse_dy -= 5; break; case MS_DOWN: mouse_dy += 5; break;
                        case MS_LEFT: mouse_dx -= 5; break; case MS_RIGHT: mouse_dx += 5; break;
                        case MS_BTN1: mouse_btn |= 0x01; break; case MS_BTN2: mouse_btn |= 0x02; break;
                        case MS_BTN3: mouse_btn |= 0x04; break; case MS_WH_U: mouse_wheel = 1; break;
                        case MS_WH_D: mouse_wheel = -1; break;
                    }
                } else {
                    // 标准按键：修饰键
                    if (hid_code >= 0xE0 && hid_code <= 0xE7) {
                        kb_report[0] |= (1 << (hid_code - 0xE0));
                    } 
                    // 标准按键：普通键
                    else if (hid_code >= 0x04 && hid_code <= 0xDF) {
                        if (key_count < 6) { 
                            kb_report[2 + key_count] = (uint8_t)hid_code; 
                            key_count++; 
                        }
                        // 填充 NKRO 位图
                        uint8_t byte_idx = (hid_code - 0x04) / 8;
                        uint8_t bit_idx = (hid_code - 0x04) % 8;
                        if (byte_idx < 31) {
                            nkro_bitmap[byte_idx] |= (1 << bit_idx);
                        }
                    }
                }
            }
        }
    }
    
    // 3. 宏触发复位
    bool any_macro_key = false;
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            if (matrix_state[r][c] && keymap[current_layer][r][c] >= MACRO_1 && keymap[current_layer][r][c] <= MACRO_10) {
                any_macro_key = true;
            }
        }
    }
    if (!any_macro_key) macro_triggered = false;

    // 4. 执行宏（宏只会修改 6KRO 报文）
    macro_task(kb_report, &key_count);

    // 5. 分发报文发送
    if (kro_mode == 1) {
        send_nkro_report(kb_report[0], nkro_bitmap);
        send_keyboard_report(0, (uint8_t[6]){0}, 0); // 清空 6KRO 防止卡键
    } else {
        send_keyboard_report(kb_report[0], &kb_report[2], key_count);
        send_nkro_report(0, (uint8_t[31]){0});       // 清空 NKRO 防止卡键
    }

    send_consumer_report(cc_report);
    send_mouse_report(mouse_btn, mouse_dx, mouse_dy, mouse_wheel);
}