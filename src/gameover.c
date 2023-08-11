#include "gameover.h"

game_state_e over_run(void) {
    // clear screen;
    clear_viewport();

#if defined(NINTENDO)
    // restore zero sprite palette
    if (DEVICE_SUPPORTS_COLOR) set_sprite_palette(0, 1, sprite_palettes);
#elif defined(SEGA)
    wait_pad_up();      // wait pad is up before drawing on sega
#endif

    // display some background
    set_attributed_bkg_tiles(FIELD_OFFSET_X + 2, 6, 15, 3, intro_map, intro_attr);

    UBYTE tmp_score_text[16];
    memcpy(tmp_score_text, "SCORE:", 6);
    uitoa(score, tmp_score_text + 6, 10);
    UBYTE len = strlen(tmp_score_text);
    for (UBYTE *pc = tmp_score_text; (*pc); pc++) {
        *pc = ascii_to_tile(*pc);
    }
    set_bkg_tiles_blank(((DEVICE_SCREEN_WIDTH - len) >> 1) + 1, 15, len, 1, tmp_score_text);
    scroll_set_pos((len & 1) ? 4 : 8);

#if defined(NINTENDO)
    wait_vbl_done();
#endif
    SHOW_SPRITES; SHOW_BKG;

#if defined(NINTENDO)
    wait_pad_up();      // wait pad is up after drawing on the game boy
#endif

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
#if defined(SEGA)
                clear_viewport();
#endif
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
                base += move_metasprite(over[j],
                                        0,
                                        base,
                                        (FIELD_OFFSET_X << 3) + DEVICE_SPRITE_PX_OFFSET_X + 52 + (j << 4),
                                        DEVICE_SPRITE_PX_OFFSET_Y + 48 + (animation[(playfield_anim + (j << 1)) & ANIM_MASK] << 1));
            }
        }

#if defined(SEGA)
    while (VCOUNTER != (SCROLL_Y_POS + (DEVICE_SCREEN_Y_OFFSET * 8)));
    __WRITE_VDP_REG(VDP_RSCX, -scroll_get_pos());
    while (VCOUNTER != (SCROLL_Y_POS + (DEVICE_SCREEN_Y_OFFSET * 8) + 16));
    __WRITE_VDP_REG(VDP_RSCX, 0);
#endif

        wait_vbl_done();
    }
}
