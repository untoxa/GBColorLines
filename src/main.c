#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <gbdk/gbdecompress.h>

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
    sound_init();

    HIDE_SPRITES; HIDE_BKG;
    SPRITES_8x16;

#if defined(NINTENDO)
    __critical {
        TMA_REG = 0xC0u; TAC_REG = 0x07u;
        LYC_REG = 0; STAT_REG |= STATF_LYC;
        add_LCD(scroll_update_isr);
        set_interrupts(VBL_IFLAG | TIM_IFLAG | LCD_IFLAG);
    }
#endif
    toggle_sound_settings(0);

#if defined(NINTENDO)
    OBP1_REG = OBP0_REG = DMG_PALETTE(DMG_WHITE, DMG_LITE_GRAY, DMG_WHITE, DMG_BLACK); 
    BGP_REG = DMG_PALETTE(DMG_WHITE, DMG_LITE_GRAY, DMG_DARK_GRAY, DMG_BLACK);
    if (DEVICE_SUPPORTS_COLOR) {
        set_bkg_palette(0, 8, background_palettes);
        set_sprite_palette(0, 8, sprite_palettes);
    }

    gb_decompress_bkg_data(0, bkg_tiles);
#ifdef CATSKULL_LOGO
    gb_decompress_bkg_data(0x40, catskull_tiles);
#endif
    gb_decompress_sprite_data(0, sprite_tiles);

    gb_decompress_bkg_data(0x80, font);
#elif defined(SEGA)
    set_bkg_palette(0, 1, background_palettes);
    set_sprite_palette(0, 1, sprite_palettes);

    uint8_t * buffer = (uint8_t *)0xD000;
    uint8_t ntiles;

    ntiles = gb_decompress(bkg_tiles, buffer) >> 4;
    set_2bpp_palette(COMPAT_PALETTE(0, 0, 12, 1));
    set_bkg_data(0, 1, buffer);
    for (uint8_t i = 1, c = 0, *buf = buffer + 16u; i < ntiles; i += 4, c++, buf += 64u) {
        set_2bpp_palette(background_compat_palettes[c]);
        set_bkg_data(i, 4, buf);
    }

    ntiles = gb_decompress(sprite_tiles, buffer) >> 4;
    for (uint8_t i = 0, c = 0, *buf = buffer; i < ntiles; i += 4, c++, buf += 64u) {
        set_2bpp_palette(sprite_compat_palettes[c]);
        set_sprite_data(i, 4, buf);
    }

    ntiles =  gb_decompress(font, buffer) >> 4;
    set_2bpp_palette(COMPAT_PALETTE(0, 0, 0, 1));
    set_bkg_data(0x40, ntiles, buffer);
    set_sprite_data(0x40, ntiles, buffer);
#endif

    memcpy(item, item_defaults, sizeof(item));
    for (UBYTE i = 0; i != PREVIEW_SIZE; i++) 
        memcpy(preview_items[i], item_defaults, sizeof(preview_items[0]));

    memcpy(score_display, score_display_defaults, sizeof(score_display));

    ENABLE_RAM;
    highscore = score_load();

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