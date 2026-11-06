#include "keymap.h"
#include "keycodes.h"
#include "macro.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>
#include <stdlib.h>

#define FLASH_TARGET_OFFSET (2 * 1024 * 1024) 

// 数据布局: [magic(2字节)] + [keymap(576字节)] + [macros(1500字节)] + [kro_mode(1字节)]
#define MAGIC_OFFSET 0
#define KEYMAP_OFFSET 2
#define MACRO_OFFSET (KEYMAP_OFFSET + sizeof(keymap))
#define KRO_OFFSET (MACRO_OFFSET + sizeof(macros))
#define TOTAL_SAVE_SIZE (KRO_OFFSET + 1)

uint8_t kro_mode = 0; // 默认 6KRO

uint16_t keymap[NUM_LAYERS][MATRIX_ROWS][MATRIX_COLS] = {
    // (保留你原有的默认键位数据，这里为了篇幅省略)
    // ...
};

void load_keymap_from_flash(void) {
    const uint8_t *flash_data = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
    uint16_t magic;
    memcpy(&magic, flash_data + MAGIC_OFFSET, sizeof(magic));

    if (magic == 0xAAAA) {
        memcpy(keymap, flash_data + KEYMAP_OFFSET, sizeof(keymap));
        memcpy(macros, flash_data + MACRO_OFFSET, sizeof(macros));
        kro_mode = flash_data[KRO_OFFSET];
    }
    
        // 🔥 强制校验，防止从 Flash 读出非法模式导致全键失灵
    if (kro_mode > 1) {
        kro_mode = 0; 
    }
}

void save_keymap_to_flash(void) {
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    
    uint32_t pages_needed = (TOTAL_SAVE_SIZE + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
    uint32_t program_size = pages_needed * FLASH_PAGE_SIZE;
    
    uint8_t *buffer = (uint8_t *)malloc(program_size);
    if (buffer) {
        memset(buffer, 0xFF, program_size);
        uint16_t magic = 0xAAAA;
        memcpy(buffer + MAGIC_OFFSET, &magic, sizeof(magic));
        memcpy(buffer + KEYMAP_OFFSET, keymap, sizeof(keymap));
        memcpy(buffer + MACRO_OFFSET, macros, sizeof(macros));
        buffer[KRO_OFFSET] = kro_mode;
        
        flash_range_program(FLASH_TARGET_OFFSET, buffer, program_size);
        free(buffer);
    }
    restore_interrupts(ints);
}