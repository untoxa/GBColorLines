#include "intro.h"

game_state_e intro_run(void) {
    // clear screen;
    clear_viewport();

    // restore zero sprite palette
#ifdef NINTENDO
    if (DEVICE_SUPPORTS_COLOR) set_sprite_palette(0, 1, sprite_palettes);
#endif

    // display some background
    set_attributed_bkg_tiles(FIELD_OFFSET_X + 2, 6, 15, 3, intro_map, intro_attr);
    #ifdef CATSKULL_LOGO
    {
        UBYTE logo_attr[13*3];
        for (UBYTE i = 0; i != sizeof(logo_attr); i++) logo_attr[i] = myrand(&r7) + 1;
        set_attributed_bkg_tiles(FIELD_OFFSET_X + 4, 1, 13, 3, catskull_map, logo_attr);
    }
    #endif

    if (highscore) {
        uitoa(highscore, score_text + 5, 10);
        UBYTE len = strlen(score_text);
        for (UBYTE *pc = score_text + 5; (*pc); pc++) {
            *pc = ascii_to_tile(*pc);
        }
#if defined(NINTENDO)
        set_bkg_tiles_blank(DEVICE_SCREEN_WIDTH - len, DEVICE_SCREEN_HEIGHT, len, 1, score_text);
#elif defined(SEGA)
        set_bkg_tiles_blank(DEVICE_SCREEN_WIDTH - len, DEVICE_SCREEN_HEIGHT - 1, len, 1, score_text);
#endif
    }

    wait_vbl_done();
    SHOW_SPRITES; SHOW_BKG;

    // loop until start
    UBYTE wait = 0;
    while (TRUE) {
        switch (joypad()) {
            case J_START:
#if defined(GAMEGEAR)
            case J_A:
#endif
                clear_viewport();
                wait_pad_up();
                wait_vbl_done();
                HIDE_SPRITES; HIDE_BKG;
                return game_play;
#ifdef NINTENDO
            case J_SELECT:
                if (wait) break;
                toggle_sound_settings(2);
                wait = 10;
                break;
#endif
        }
        // animate screen
        if (sys_time & 1) {
            playfield_anim++; playfield_anim &= ANIM_MASK;
            UBYTE base = move_metasprite(start_msg,
                                         0,
                                         0,
                                         DEVICE_SPRITE_PX_OFFSET_X + ((DEVICE_SCREEN_WIDTH - START_MSG_WIDTH) << 2) + animation[playfield_anim],
                                         DEVICE_SPRITE_PX_OFFSET_Y + 88);
            for (UBYTE j = 0; j != TITLE_SIZE; j++) {
                base += move_metasprite(title[j],
                                        0,
                                        base,
                                        (FIELD_OFFSET_X << 3) + DEVICE_SPRITE_PX_OFFSET_X + 44 + (j << 4),
                                        DEVICE_SPRITE_PX_OFFSET_Y + 48 + (animation[(playfield_anim + (j << 1)) & ANIM_MASK] << 1));
            }
        }
        // process delay
        if (wait) wait--;

        // process scroll
        scroll_process();

        wait_vbl_done();
    }
}
