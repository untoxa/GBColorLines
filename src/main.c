#include <gb/gb.h>
#include <gb/metasprites.h>
#include <gb/gbdecompress.h>

#include <string.h>
#include <stdlib.h>

#include "game_types.h"
#include "common_utils.h"
#include "scroller.h"
#include "score.h"
#include "playfield.h"
#include "sound.h"
#include "graphics.h"
#include "myrand.h"

// game states:
#include "intro.h"
#include "gameover.h"
#include "game.h"

game_state_e game_state = game_intro;

void main() {
    NR52_REG = 0x80u;
    NR51_REG = 0xffu;
    NR50_REG = 0x77u;

    HIDE_SPRITES; HIDE_BKG;

    __critical {
        TMA_REG = 0xC0u; TAC_REG = 0x07u;
        add_TIM(music_update);
        LYC_REG = 0; STAT_REG |= 0b01000000;
        add_LCD(scroll_update_isr);
        set_interrupts(VBL_IFLAG | TIM_IFLAG | LCD_IFLAG);
    }
    toggle_sound_settings(0);

    OBP1_REG = OBP0_REG = DMG_PALETTE(DMG_WHITE, DMG_LITE_GRAY, DMG_WHITE, DMG_BLACK); 
    BGP_REG = DMG_PALETTE(DMG_WHITE, DMG_LITE_GRAY, DMG_DARK_GRAY, DMG_BLACK);

    if (_cpu == CGB_TYPE) {
        set_bkg_palette(0, 8, background_palettes);
        set_sprite_palette(0, 8, sprite_palettes);
    }
    gb_decompress_bkg_data(0, bkg_tiles);
    #ifdef CATSKULL_LOGO
    gb_decompress_bkg_data(0x40, catskull_tiles);
    #endif
    gb_decompress_sprite_data(0, sprite_tiles);

    gb_decompress_bkg_data(0x80, font);

    memcpy(item, item_defaults, sizeof(item));
    for (UBYTE i = 0; i != PREVIEW_SIZE; i++) 
        memcpy(preview_items[i], item_defaults, sizeof(preview_items[0]));

    memcpy(score_display, score_display_defaults, sizeof(score_display));

    ENABLE_RAM_MBC5;
    highscore = score_load();

    SPRITES_8x16;

    randomize();
    myrand_init(7, &r7);

    while (TRUE) {
        switch (game_state) {
            case game_intro:
                game_state = intro_run();
                break;
            case game_play:
                game_state = game_run();
                if (highscore < score) highscore = score_save(score);
                break;
            case game_over:
                game_state = over_run();
                break;
        }
    }
}