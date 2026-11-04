#ifndef _MATRIX_H_
#define _MATRIX_H_

#include <stdint.h>
#include <stdbool.h>

#define MATRIX_ROWS 8
#define MATRIX_COLS 18

extern bool matrix_state[MATRIX_ROWS][MATRIX_COLS];
extern uint8_t current_layer;

void matrix_init(void);
void matrix_scan(void);

#endif // _MATRIX_H_