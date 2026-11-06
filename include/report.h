#ifndef _REPORT_H_
#define _REPORT_H_

#include <stdint.h>

void send_keyboard_report(uint8_t modifier, uint8_t *keys, uint8_t count);
void send_nkro_report(uint8_t modifier, uint8_t *bitmap);
void send_consumer_report(uint16_t code);
void send_mouse_report(uint8_t btn, int8_t dx, int8_t dy, int8_t wheel);

#endif // _REPORT_H_