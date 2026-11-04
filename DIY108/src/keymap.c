#include "keymap.h"
#include "keycodes.h"
#include "macro.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>
#include <stdlib.h>

// 利用 16MB 空间，配置区放在 2MB 处
#define FLASH_TARGET_OFFSET (2 * 1024 * 1024) 

// 🔥 使用纯字节偏移，彻底避开 C 语言结构体对齐的坑
// 数据布局: [magic(2字节)] + [keymap(576字节)] + [macros(1500字节)]
#define MAGIC_OFFSET 0
#define KEYMAP_OFFSET 2
#define MACRO_OFFSET (KEYMAP_OFFSET + sizeof(keymap))
#define TOTAL_SAVE_SIZE (MACRO_OFFSET + sizeof(macros))

uint16_t keymap[NUM_LAYERS][MATRIX_ROWS][MATRIX_COLS] = {
    // Layer 0
    {
        {HID_LCTL, KC_GRV,  KC_F1,   KC_F2,   HID_NONE, KC_5,    KC_6,    KC_EQL,  KC_F8,   KC_MINS, HID_NONE, KC_F9,   CC_MUTE,  HID_NONE, KC_DEL,  KC_HOME, KC_INS,  KC_PGUP},
        {KC_F5,    KC_1,    KC_2,    KC_3,    KC_PSCR, KC_4,    KC_7,    KC_8,    KC_9,    KC_0,    HID_NONE, KC_F10,  HID_NONE, HID_NONE, HID_NONE, KC_END,  HID_NONE, KC_PGDN},
        {KC_PAUS,  KC_Q,    KC_W,    KC_E,    KC_SCRL, KC_R,    KC_U,    KC_I,    KC_O,    KC_P,    HID_NONE, HID_NONE, HID_NONE, HID_NONE, KC_P7,   KC_PPLS, KC_P8,   KC_P9},
        {HID_NONE, KC_A,    KC_S,    KC_D,    MO_FN,   KC_F,    KC_J,    KC_K,    KC_L,    KC_SCLN, HID_RGUI, KC_BSLS, HID_NONE, HID_RSFT, KC_P1,   KC_PENT, KC_P2,   KC_P3},
        {HID_RCTL, KC_Z,    KC_X,    KC_C,    HID_NONE, KC_V,    KC_M,    KC_COMM, KC_DOT,  HID_NONE, HID_NONE, KC_ENT,  HID_NONE, CC_VOLU, KC_NUM,  HID_NONE, KC_PSLS, KC_PAST},
        {HID_NONE, KC_TAB,  KC_CAPS, KC_F3,   HID_NONE, KC_T,    KC_Y,    KC_RBRC, KC_F7,   KC_LBRC, HID_NONE, KC_BSPC, HID_LGUI, HID_LSFT, KC_P4,   HID_NONE, KC_P5,   KC_P6},
        {HID_NONE, KC_ESC,  HID_LALT, KC_F4,   HID_LALT, KC_G,    KC_H,    KC_F6,   HID_NONE, KC_QUOT, HID_NONE, KC_F11,  HID_NONE, CC_VOLD, KC_SPC,  KC_UP,   KC_P0,   KC_PDOT},
        {HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_RALT, KC_B,  KC_N,    HID_NONE, HID_NONE, KC_SLSH, CC_CALC, KC_F12,  HID_NONE, HID_NONE, KC_DOWN, KC_LEFT, KC_RGHT, KC_PMNS}
    },
    // Layer 1 (FN)
    {
        {CC_MUTE,  CC_VOLD, CC_VOLU, CC_MPLY, CC_MPRV, CC_MNXT, CC_STOP, CC_BRIU, CC_BRID, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE},
        {CC_WBAK,  CC_WFWD, CC_WHOM, CC_WSCH, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE},
        {HID_NONE, HID_NONE, MS_UP,   HID_NONE, HID_NONE, MS_BTN1, MS_BTN2, MS_BTN3, MS_WH_U, MS_WH_D, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE},
        {HID_NONE, MS_LEFT, MS_DOWN, MS_RIGHT, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE},
        {HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE},
        {HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE},
        {CC_PWR,   CC_SLEP, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE},
        {HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE, HID_NONE}
    }
};

void load_keymap_from_flash(void) {
    const uint8_t *flash_data = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
    uint16_t magic;
    memcpy(&magic, flash_data + MAGIC_OFFSET, sizeof(magic));

    // 去掉时间戳校验，只要魔术字正确就直接读取
    if (magic == 0xAAAA) {
        memcpy(keymap, flash_data + KEYMAP_OFFSET, sizeof(keymap));
        memcpy(macros, flash_data + MACRO_OFFSET, sizeof(macros));
    }
}

void save_keymap_to_flash(void) {
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    
    uint32_t pages_needed = (TOTAL_SAVE_SIZE + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
    uint32_t program_size = pages_needed * FLASH_PAGE_SIZE;
    
    uint8_t *buffer = (uint8_t *)malloc(program_size);
    if (buffer) {
        memset(buffer, 0xFF, program_size); // 擦除后默认是 0xFF
        uint16_t magic = 0xAAAA;
        memcpy(buffer + MAGIC_OFFSET, &magic, sizeof(magic));
        memcpy(buffer + KEYMAP_OFFSET, keymap, sizeof(keymap));
        memcpy(buffer + MACRO_OFFSET, macros, sizeof(macros));
        
        flash_range_program(FLASH_TARGET_OFFSET, buffer, program_size);
        free(buffer);
    }
    restore_interrupts(ints);
}