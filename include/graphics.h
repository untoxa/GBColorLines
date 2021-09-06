#ifndef _GRAPHICS_H_INCLUDE
#define _GRAPHICS_H_INCLUDE

#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#include "game_types.h"

#ifdef CATSKULL_LOGO
extern const unsigned char catskull_tiles[];
extern const unsigned char catskull_map[];
#endif

extern const unsigned char sprite_tiles[];
extern const unsigned char bkg_tiles[];

extern const unsigned char intro_map[];
extern const unsigned char intro_attr[];

extern const unsigned char field_row[];
extern const unsigned char field_item_empty[];

extern const palette_entry_t background_palettes[];
extern const palette_entry_t sprite_palettes[];
#ifdef SEGA
extern const uint16_t background_compat_palettes[];
extern const uint16_t sprite_compat_palettes[];
#endif

extern const metasprite_t cursor[];
extern const metasprite_t item_defaults[];

extern const unsigned char font[];

#define TITLE_SIZE 5
extern const metasprite_t * const title[];
extern const metasprite_t start_msg[];

#define OVER_SIZE 4
extern const metasprite_t * const over[];

extern const metasprite_t score_display_defaults[];

#define SCORE_ANIM_SIZE 16 * 8
extern const UBYTE score_animation[SCORE_ANIM_SIZE];

#if defined(NINTENDO)
#define ASCII_TO_TILE(ch) (UBYTE)(((ch) > 0x20) ? (((ch) - 0x21) << 1) + 0x80 : 0u)
#elif defined(SEGA)
#define ASCII_TO_TILE(ch) (UBYTE)(((ch) > 0x20) ? (((ch) - 0x21) << 1) + 0x40 : 0u)
#endif

inline void set_bkg_tiles_blank(UBYTE x, UBYTE y, UBYTE w, UBYTE h, const UBYTE * map) {
#ifdef NINTENDO
    if (DEVICE_SUPPORTS_COLOR) {
        VBK_REG = 1;
        fill_bkg_rect(x, y, w, h, 0);
        VBK_REG = 0;
    }
#endif
    set_bkg_tiles(x, y, w, h, (UBYTE *)map);
}

inline void set_attributed_bkg_tiles(UBYTE x, UBYTE y, UBYTE w, UBYTE h, const UBYTE * map, const UBYTE * attr) {
#ifdef NINTENDO
    if (DEVICE_SUPPORTS_COLOR) {
        VBK_REG = 1;
        set_bkg_tiles(x, y, w, h, (UBYTE *)attr);
        VBK_REG = 0;
    }
#else
    attr;
#endif
    set_bkg_tiles(x, y, w, h, (UBYTE *)map);
}

inline void set_attributed_bkg_tile_xy(UBYTE x, UBYTE y, UBYTE t, UBYTE a) {
#ifdef NINTENDO
    if (DEVICE_SUPPORTS_COLOR) {
        VBK_REG = 1;
        set_bkg_tile_xy(x, y, a);
        VBK_REG = 0;
    }
#else 
    a;
#endif
    set_bkg_tile_xy(x, y, t);
}

#endif