#ifndef _KEYMAP_H_
#define _KEYMAP_H_

#include <stdint.h>
#include "matrix.h"

#define NUM_LAYERS 2

extern uint16_t keymap[NUM_LAYERS][MATRIX_ROWS][MATRIX_COLS];

void load_keymap_from_flash(void);
void save_keymap_to_flash(void);

#endif // _KEYMAP_H_