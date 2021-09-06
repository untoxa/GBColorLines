#include "gameover.h"

game_state_e over_run() {
    // clear screen;
    clear_viewport();

    // restore zero sprite palette
#ifdef NINTENDO
    if (DEVICE_SUPPORTS_COLOR) set_sprite_palette(0, 1, sprite_palettes);
#endif

    // display some background
    set_attributed_bkg_tiles(2, 6, 15, 3, intro_map, intro_attr);

    UBYTE tmp_score_text[16];
    memcpy(tmp_score_text, "SCORE:", 6);
    uitoa(score, tmp_score_text + 6, 10);
    UBYTE len = strlen(tmp_score_text);
    for (UBYTE *pc = tmp_score_text; (*pc); pc++) {
        *pc = ascii_to_tile(*pc);
    }
    set_bkg_tiles_blank(((DEVICE_SCREEN_WIDTH - len) >> 1) + 1, 15, len, 1, tmp_score_text); 
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
                base += move_metasprite(over[j], 0, base, DEVICE_SPRITE_OFFSET_X + 52 + (j << 4), DEVICE_SPRITE_OFFSET_X + 48 + (animation[(playfield_anim + (j << 1)) & ANIM_MASK] << 1));
            }
        }
        wait_vbl_done();
    }
}
