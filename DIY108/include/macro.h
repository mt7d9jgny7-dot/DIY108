#ifndef _MACRO_H_
#define _MACRO_H_

#include <stdint.h>
#include <stdbool.h>

#define MAX_MACROS 10
#define MAX_MACRO_STEPS 50

#define MACRO_END   0
#define MACRO_DOWN  1
#define MACRO_UP    2
#define MACRO_DELAY 3

// 关键修复：使用 __attribute__((packed)) 强制取消 4 字节对齐，确保结构体严格为 3 字节
typedef struct __attribute__((packed)) {
    uint8_t type;
    uint16_t value;
} macro_action_t;

extern macro_action_t macros[MAX_MACROS][MAX_MACRO_STEPS];

void macro_init(void);
void play_macro(uint8_t id);
bool macro_task(uint8_t* kb_report, uint8_t* key_count);

#endif // _MACRO_H_