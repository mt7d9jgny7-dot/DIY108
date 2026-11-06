#include "webhid.h"
#include "tusb.h"
#include "keymap.h"
#include "macro.h"
#include "pico/time.h"
#include "pico/bootrom.h"
#include <string.h>

#define TOTAL_WEBHID_SIZE (sizeof(keymap) + sizeof(macros) + 1)

static uint8_t webhid_rx_buffer[2500];
static uint16_t webhid_rx_len = 0;
static uint64_t save_timer = 0;
static uint32_t webhid_tx_offset = 0;

// 处理延迟写入 Flash
void webhid_task(void) {
    if (save_timer != 0 && time_us_64() > save_timer) {
        save_timer = 0;
        save_keymap_to_flash();
    }
}

// 处理来自网页的读取请求
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

// 处理来自网页的写入/控制请求
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    // Report ID 6：重启进入 Bootloader
    if (report_id == 6 && report_type == HID_REPORT_TYPE_FEATURE) {
        reset_usb_boot(0, 0);
        return;
    }

    // Report ID 4：接收网页配置数据
    if (report_id == 4 && report_type == HID_REPORT_TYPE_FEATURE) {
        if (bufsize == 65 && buffer[0] == 0x04) { buffer++; bufsize--; }
        
        if (webhid_rx_len + bufsize <= sizeof(webhid_rx_buffer)) {
            memcpy(webhid_rx_buffer + webhid_rx_len, buffer, bufsize);
            webhid_rx_len += bufsize;
            
            if (webhid_rx_len >= TOTAL_WEBHID_SIZE) {
                memcpy(keymap, webhid_rx_buffer, sizeof(keymap));
                memcpy(macros, webhid_rx_buffer + sizeof(keymap), sizeof(macros));
                kro_mode = webhid_rx_buffer[sizeof(keymap) + sizeof(macros)];
                if (kro_mode > 1) kro_mode = 0; // 容错
                
                webhid_rx_len = 0; 
                save_timer = time_us_64() + 500000; // 延迟 500ms 写 Flash
            }
        } else { 
            webhid_rx_len = 0; 
        }
    }
}