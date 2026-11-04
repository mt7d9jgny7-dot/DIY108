#include "macro.h"
#include "pico/time.h"

macro_action_t macros[MAX_MACROS][MAX_MACRO_STEPS] = {0};

static bool is_playing = false;
static uint8_t current_macro = 0;
static uint8_t current_step = 0;
static uint64_t wait_until = 0;

// 记录宏当前按下的键，用于结束时强制释放
static uint8_t macro_pressed_keys[6] = {0};
static uint8_t macro_pressed_count = 0;

void macro_init(void) {
    is_playing = false;
    macro_pressed_count = 0;
}

void play_macro(uint8_t id) {
    if (id >= MAX_MACROS) return;
    is_playing = true;
    current_macro = id;
    current_step = 0;
    wait_until = 0;
    macro_pressed_count = 0;
}

// 辅助函数：检查宏里是否已经包含了抬起动作
static bool is_key_up_in_macro(uint8_t macro_id, uint16_t key_code) {
    for (int i = 0; i < MAX_MACRO_STEPS; i++) {
        if (macros[macro_id][i].type == MACRO_UP && macros[macro_id][i].value == key_code) {
            return true;
        }
    }
    return false;
}

bool macro_task(uint8_t* kb_report, uint8_t* key_count) {
    if (!is_playing) return false;

    if (current_step >= MAX_MACRO_STEPS) {
        is_playing = false;
        // 宏异常结束时，强制清空它按下的按键，避免卡死物理键盘
        for(int i=0; i<macro_pressed_count; i++) {
            for(int j=0; j<6; j++) {
                if(kb_report[2+j] == macro_pressed_keys[i]) {
                    kb_report[2+j] = 0;
                }
            }
        }
        return false;
    }

    if (time_us_64() >= wait_until) {
        macro_action_t act = macros[current_macro][current_step];
        
        if (act.type == MACRO_END) {
            is_playing = false;
            // 结束时也强制清空宏按下的按键
            for(int i=0; i<macro_pressed_count; i++) {
                for(int j=0; j<6; j++) {
                    if(kb_report[2+j] == macro_pressed_keys[i]) {
                        kb_report[2+j] = 0;
                    }
                }
            }
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
                if (macro_pressed_count < 6) {
                    macro_pressed_keys[macro_pressed_count++] = (uint8_t)code;
                }
                (*key_count)++;
            }
            current_step++;
            return true;
        } 
        else if (act.type == MACRO_UP) {
            uint16_t code = act.value;
            // 在普通按键列表里寻找并移除它
            for (int i = 0; i < 6; i++) {
                if (kb_report[2 + i] == (uint8_t)code) {
                    kb_report[2 + i] = 0;
                }
            }
            // 如果都是空的，把后面的往前移，填补空隙
            uint8_t new_kc = 0;
            for (int i = 0; i < 6; i++) {
                if (kb_report[2 + i] != 0) {
                    kb_report[2 + new_kc++] = kb_report[2 + i];
                }
            }
            for(int i=new_kc; i<6; i++) kb_report[2+i] = 0;
            *key_count = new_kc;

            if (code >= 0xE0 && code <= 0xE7) {
                kb_report[0] &= ~(1 << (code - 0xE0));
            }
            current_step++;
            return true;
        }
        
        current_step++;
        return true;
    }
    return true; 
}