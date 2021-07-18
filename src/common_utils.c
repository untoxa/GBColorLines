#include "common_utils.h"

void wait_pad_up() {
    if (joypad()) {
        do {
            wait_vbl_done();        
        } while (joypad());
    }
}

void clear_screen() {
    if (_cpu == CGB_TYPE) {
        VBK_REG = 1;
        fill_bkg_rect(0, 0, 22, 20, 0);
        VBK_REG = 0;
    }
    fill_bkg_rect(0, 0, 22, 20, 0);
}

void clear_viewport() {
    OAM_item_t * ptr = shadow_OAM;
    for (UBYTE i = 0; i != 40; i++) ptr->y = 0, ptr++;    
    clear_screen();
    scroll_reset();
}
