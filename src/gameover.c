#include "gameover.h"

game_state_e over_run() {
    // clear screen;
    clear_viewport();
    // restore zero sprite palette
    if (_cpu == CGB_TYPE) set_sprite_palette(0, 1, sprite_palettes);

    // display some background
    set_attributed_bkg_tiles(2, 6, 15, 3, intro_map, intro_attr);

    UBYTE tmp_score_text[16];
    memcpy(tmp_score_text, "SCORE:", 6);
    UBYTE *pc = tmp_score_text;
    UBYTE len = strlen(utoa(score, tmp_score_text + 6)) + 6;
    while (*pc) {
        *pc++ = ((*pc - 0x21) << 1) + 0x80;
    }
    set_bkg_tiles_blank(((20 - len) >> 1) + 1, 15, len, 1, tmp_score_text); 
    scroll_set_pos((len & 1) ? 4 : 8);

    wait_vbl_done();
    SHOW_SPRITES; SHOW_BKG;

    // wait pad is up
    wait_pad_up();

    UBYTE wait = 0;
    while (TRUE) {
        switch (joypad()) {
            case 0:
                break;
            case J_SELECT:
                if (wait) break;
                toggle_sound_settings(2);
                wait = 10;
                break;
            default: 
                wait_pad_up();
                wait_vbl_done();
                HIDE_SPRITES; HIDE_BKG;
                return game_intro;
        }
        // process delay
        if (wait) wait--;

        // animate screen
        if (sys_time & 1) {
            playfield_anim++; playfield_anim &= ANIM_MASK;
            UBYTE base = 0;
            for (UBYTE j = 0; j != OVER_SIZE; j++) {
                base += move_metasprite(over[j], 0, base, (j << 4) + 60, 64 + (animation[(playfield_anim + (j << 1)) & ANIM_MASK] << 1));
            }
        }
        wait_vbl_done();
    }
}
