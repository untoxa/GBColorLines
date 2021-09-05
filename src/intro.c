#include "intro.h"

game_state_e intro_run() {
    // clear screen;
    clear_viewport();

    // restore zero sprite palette
#ifdef NINTENDO
    if (DEVICE_SUPPORTS_COLOR) set_sprite_palette(0, 1, sprite_palettes);
#endif

    // display some background
    set_attributed_bkg_tiles(2, 6, 15, 3, intro_map, intro_attr);
    #ifdef CATSKULL_LOGO
    {
        UBYTE logo_attr[13*3];
        for (UBYTE i = 0; i != sizeof(logo_attr); i++) logo_attr[i] = myrand(&r7) + 1; 
        set_attributed_bkg_tiles(4, 1, 13, 3, catskull_map, logo_attr);
    }
    #endif

    UBYTE start_sprite = move_metasprite(start_msg, 0, 0, 42, 104);

    if (highscore) {
        UBYTE *pc = score_text + 5;
        UBYTE len = strlen(uitoa(highscore, pc, 10));
        while (*pc) {
            *pc++ = ((*pc - 0x21) << 1) + 0x80;
        }
        set_bkg_tiles_blank(15 - len, 18, 5 + len, 1, score_text); 
    }

    wait_vbl_done();
    SHOW_SPRITES; SHOW_BKG;

    // loop until start
    UBYTE wait = 0;
    while (TRUE) {
        switch (joypad()) {
#if defined(NINTENDO)
            case J_START:
#elif defined(SEGA)
            case J_A:
#endif 
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
            move_metasprite(start_msg, 0, 0, 42 + animation[playfield_anim], 104);
            UBYTE base = start_sprite;
            for (UBYTE j = 0; j != TITLE_SIZE; j++) {
                base += move_metasprite(title[j], 0, base, (j << 4) + 52, 64 + (animation[(playfield_anim + (j << 1)) & ANIM_MASK] << 1));
            }
        }
        // process delay
        if (wait) wait--;

        wait_vbl_done();
        // process scroll
        scroll_process();
    }
}
