#include "macro.h"
#include "pico/time.h"

macro_action_t macros[MAX_MACROS][MAX_MACRO_STEPS] = {0};

static bool is_playing = false;
static uint8_t current_macro = 0;
static uint8_t current_step = 0;
static uint64_t wait_until = 0;

void macro_init(void) {
    is_playing = false;
}

void play_macro(uint8_t id) {
    if (id >= MAX_MACROS) return;
    is_playing = true;
    current_macro = id;
    current_step = 0;
    wait_until = 0;
}

bool macro_task(uint8_t* kb_report, uint8_t* key_count) {
    if (!is_playing) return false;

    // 越界保护：如果当前步骤超出了最大步数，强制结束
    if (current_step >= MAX_MACRO_STEPS) {
        is_playing = false;
        return false;
    }

    if (time_us_64() >= wait_until) {
        macro_action_t act = macros[current_macro][current_step];
        
        if (act.type == MACRO_END) {
            is_playing = false;
            return false;
        } 
        else if (act.type == MACRO_DELAY) {
            wait_until = time_us_64() + (act.value * 1000ULL);
            current_step++;
            return true;
        } 
        else if (act.type == MACRO_DOWN) {
            uint16_t code = act.value;
            if (code >= 0xE0 && code <= 0xE7) {
                kb_report[0] |= (1 << (code - 0xE0));
            } else if (*key_count < 6) {
                kb_report[2 + (*key_count)] = (uint8_t)code;
                (*key_count)++;
            }
            current_step++;
            return true;
        } 
        else if (act.type == MACRO_UP) {
            current_step++;
            return true;
        }
        
        // 🔥 如果读到未知类型（异常数据），强制跳过该步，防止死循环
        current_step++;
        return true;
    }
    return true; 
}