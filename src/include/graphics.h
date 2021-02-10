#ifndef _GRAPHICS_H_INCLUDE
#define _GRAPHICS_H_INCLUDE

#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>

extern const unsigned char sprite_tiles[];
extern const unsigned char bkg_tiles[];

extern const unsigned char intro_map[];
extern const unsigned char intro_attr[];

extern const unsigned char field_row[];
extern const unsigned char field_item_empty[];

extern const unsigned int palettes[];

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

#define ASCII_TO_TILE(ch) (UBYTE)(((ch) > 0x20) ? (((ch) - 0x21) << 1) + 0x80 : 0u)

#endif