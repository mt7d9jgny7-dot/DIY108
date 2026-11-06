#include "report.h"
#include "tusb.h"
#include <string.h>

void send_keyboard_report(uint8_t modifier, uint8_t *keys, uint8_t count) {
    static uint8_t prev_mod = 0;
    static uint8_t prev_keys[6] = {0};
    
    // 检查是否发生变化，避免重复发包
    if (modifier == prev_mod && memcmp(keys, prev_keys, 6) == 0) return;

    if (tud_hid_ready()) {
        tud_hid_keyboard_report(1, modifier, keys);
        prev_mod = modifier;
        memcpy(prev_keys, keys, 6);
    }
}

void send_nkro_report(uint8_t modifier, uint8_t *bitmap) {
    static uint8_t prev_mod = 0;
    static uint8_t prev_bitmap[31] = {0};
    
    if (modifier == prev_mod && memcmp(bitmap, prev_bitmap, 31) == 0) return;

    if (tud_hid_ready()) {
        uint8_t report[32];
        report[0] = modifier;
        memcpy(&report[1], bitmap, 31);
        tud_hid_report(5, report, 32);
        
        prev_mod = modifier;
        memcpy(prev_bitmap, bitmap, 31);
    }
}

void send_consumer_report(uint16_t code) {
    static uint16_t prev_code = 0;
    if (code == prev_code) return;

    if (tud_hid_ready()) {
        tud_hid_report(3, &code, sizeof(code));
        prev_code = code;
    }
}

void send_mouse_report(uint8_t btn, int8_t dx, int8_t dy, int8_t wheel) {
    if (btn == 0 && dx == 0 && dy == 0 && wheel == 0) return;

    if (tud_hid_ready()) {
        tud_hid_mouse_report(2, btn, dx, dy, wheel, 0);
    }
}