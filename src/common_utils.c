#include "common_utils.h"

void wait_pad_up() {
    if (joypad()) {
        do {
            wait_vbl_done();        
        } while (joypad());
    }
}

void clear_screen() {
#ifdef NINTENDO
    if (DEVICE_SUPPORTS_COLOR) {
        VBK_REG = 1;
        fill_bkg_rect(0, 0, DEVICE_SCREEN_BUFFER_WIDTH, DEVICE_SCREEN_HEIGHT + 2, 0);
        VBK_REG = 0;
    }
#endif
    fill_bkg_rect(0, 0, DEVICE_SCREEN_BUFFER_WIDTH, DEVICE_SCREEN_HEIGHT + 2, 0);
}

void clear_viewport() {
    hide_sprites_range(0, MAX_HARDWARE_SPRITES);    
    clear_screen();
    scroll_reset();
}
