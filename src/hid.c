#include "hid.h"
#include "matrix.h"
#include "keymap.h"
#include "keycodes.h"
#include "macro.h"
#include <string.h>
#include <stdlib.h>
#include "pico/time.h"
#include "pico/bootrom.h"

#define TOTAL_WEBHID_SIZE (sizeof(keymap) + sizeof(macros) + 1)
static uint8_t webhid_rx_buffer[2500];
static uint16_t webhid_rx_len = 0;
static uint64_t save_timer = 0;

void hid_task(void) {
    if (save_timer != 0 && time_us_64() > save_timer) {
        save_timer = 0;
        save_keymap_to_flash();
    }

    uint8_t kb_report[8] = {0}; 
    uint8_t nkro_report[32] = {0}; 
    uint16_t cc_report = 0;
    uint8_t key_count = 0;
    int8_t mouse_dx = 0, mouse_dy = 0;
    uint8_t mouse_btn = 0;
    int8_t mouse_wheel = 0;
    
    static uint8_t prev_kb_report[8] = {0};
    static uint8_t prev_nkro_report[32] = {0};
    static uint16_t prev_cc_report = 0;
    static bool macro_triggered = false;

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
                    if (hid_code >= 0xE0 && hid_code <= 0xE7) {
                        kb_report[0] |= (1 << (hid_code - 0xE0));
                        nkro_report[0] |= (1 << (hid_code - 0xE0));
                    } else if (hid_code != HID_NONE && hid_code != MO_FN) {
                        if (key_count < 6) { 
                            kb_report[2 + key_count] = (uint8_t)hid_code; 
                            key_count++; 
                        }
                        // 🔥 修复位图偏移：从第 1 字节开始算（第 0 字节是 Modifier）
                        if (hid_code > 0 && hid_code < 248) {
                            uint8_t byte_idx = 1 + (hid_code / 8);
                            uint8_t bit_idx = hid_code % 8;
                            if(byte_idx < 32) nkro_report[byte_idx] |= (1 << bit_idx);
                        }
                    }
                }
            }
        }
    }
    
    bool any_macro_key = false;
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            if (matrix_state[r][c] && keymap[current_layer][r][c] >= MACRO_1 && keymap[current_layer][r][c] <= MACRO_10) {
                any_macro_key = true;
            }
        }
    }
    if (!any_macro_key) macro_triggered = false;

    macro_task(kb_report, &key_count);

    if (!tud_hid_ready()) return;
    
    // 🔥 严格根据 kro_mode 发送报文
    if (kro_mode == 1) {
        if (memcmp(nkro_report, prev_nkro_report, 32) != 0) {
            tud_hid_report(5, nkro_report, 32);
            memcpy(prev_nkro_report, nkro_report, 32);
        }
    } else {
        // kro_mode 非 1 的时候强制走 6KRO，保证不会失联
        if (memcmp(kb_report, prev_kb_report, 8) != 0) {
            tud_hid_keyboard_report(1, kb_report[0], &kb_report[2]);
            memcpy(prev_kb_report, kb_report, 8);
        }
    }

    if (cc_report != prev_cc_report) {
        tud_hid_report(3, &cc_report, sizeof(cc_report));
        prev_cc_report = cc_report;
    }
    if (mouse_dx || mouse_dy || mouse_btn || mouse_wheel) {
        tud_hid_mouse_report(2, mouse_btn, mouse_dx, mouse_dy, mouse_wheel, 0);
    }
}

static uint32_t webhid_tx_offset = 0;

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    if (report_id == 4 && report_type == HID_REPORT_TYPE_FEATURE) {
        if (webhid_tx_offset >= TOTAL_WEBHID_SIZE) webhid_tx_offset = 0; 
        uint16_t copy_len = 64;
        if (webhid_tx_offset + copy_len > TOTAL_WEBHID_SIZE) {
            copy_len = TOTAL_WEBHID_SIZE - webhid_tx_offset;
        }
        uint16_t copied = 0;
        if (webhid_tx_offset < sizeof(keymap)) {
            uint16_t rem = sizeof(keymap) - webhid_tx_offset;
            uint16_t take = rem < copy_len ? rem : copy_len;
            memcpy(buffer, (uint8_t *)keymap + webhid_tx_offset, take);
            copied += take;
        }
        if (copied < copy_len) {
            uint16_t macro_offset = 0;
            if (webhid_tx_offset + copied >= sizeof(keymap)) {
                macro_offset = (webhid_tx_offset + copied) - sizeof(keymap);
            }
            if (macro_offset < sizeof(macros)) {
                uint16_t rem = sizeof(macros) - macro_offset;
                uint16_t take = rem < (copy_len - copied) ? rem : (copy_len - copied);
                memcpy(buffer + copied, (uint8_t *)macros + macro_offset, take);
                copied += take;
            }
        }
        if (copied < copy_len) {
            buffer[copied] = kro_mode;
            copied++;
        }
        webhid_tx_offset += copy_len;
        return copy_len;
    }
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    if (report_id == 6 && report_type == HID_REPORT_TYPE_FEATURE) {
        reset_usb_boot(0, 0);
        return;
    }

    if (report_id == 4 && report_type == HID_REPORT_TYPE_FEATURE) {
        if (bufsize == 65 && buffer[0] == 0x04) { buffer++; bufsize--; }
        if (webhid_rx_len + bufsize <= sizeof(webhid_rx_buffer)) {
            memcpy(webhid_rx_buffer + webhid_rx_len, buffer, bufsize);
            webhid_rx_len += bufsize;
            if (webhid_rx_len >= TOTAL_WEBHID_SIZE) {
                memcpy(keymap, webhid_rx_buffer, sizeof(keymap));
                memcpy(macros, webhid_rx_buffer + sizeof(keymap), sizeof(macros));
                kro_mode = webhid_rx_buffer[sizeof(keymap) + sizeof(macros)];
                if (kro_mode > 1) kro_mode = 0; // 容错防止卡死
                webhid_rx_len = 0; 
                save_timer = time_us_64() + 500000; 
            }
        } else { webhid_rx_len = 0; }
    }
}