#include "matrix.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

const uint8_t ROWS[] = {0, 1, 2, 3, 4, 5, 6, 7};
const uint8_t COLS[] = {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 26, 27, 28};

#define DEBOUNCE_TIME_MS 5

bool matrix_state[MATRIX_ROWS][MATRIX_COLS] = {false};
uint8_t current_layer = 0;

static uint32_t last_matrix_state[MATRIX_ROWS][MATRIX_COLS] = {0};

void matrix_init(void) {
    for (int i = 0; i < MATRIX_ROWS; i++) {
        gpio_init(ROWS[i]); 
        gpio_set_dir(ROWS[i], GPIO_OUT); 
        gpio_put(ROWS[i], 0);
    }
    for (int i = 0; i < MATRIX_COLS; i++) {
        gpio_init(COLS[i]); 
        gpio_set_dir(COLS[i], GPIO_IN); 
        gpio_pull_down(COLS[i]);
    }
}

void matrix_scan(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    for (int r = 0; r < MATRIX_ROWS; r++) {
        gpio_put(ROWS[r], 1);
        sleep_us(5);
        for (int c = 0; c < MATRIX_COLS; c++) {
            bool pressed = gpio_get(COLS[c]);
            if (pressed != matrix_state[r][c]) {
                if (now - last_matrix_state[r][c] > DEBOUNCE_TIME_MS) {
                    matrix_state[r][c] = pressed;
                    last_matrix_state[r][c] = now;
                    
                    if (r == 3 && c == 4) {
                        current_layer = pressed ? 1 : 0;
                    }
                }
            } else {
                last_matrix_state[r][c] = now;
            }
        }
        gpio_put(ROWS[r], 0);
    }
}