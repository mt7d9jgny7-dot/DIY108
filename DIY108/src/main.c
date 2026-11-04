#include <stdio.h>
#include "pico/stdlib.h"
#include "tusb.h"

#include "matrix.h"
#include "keymap.h"
#include "hid.h"
#include "macro.h"

int main() {
    stdio_init_all();
    load_keymap_from_flash(); 
    macro_init();
    tusb_init();
    matrix_init();

    uint32_t last_mouse_time = 0;

    while (1) {
        tud_task();        
        matrix_scan();     
        
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_mouse_time > 10) {
            hid_task();    
            last_mouse_time = now;
        }
        
        sleep_us(500);     
    }
    return 0;
}