#ifndef _PLAYFIELD_H_INCLUDE
#define _PLAYFIELD_H_INCLUDE

#include <gb/gb.h>
#include <gb/metasprites.h>

#include "game_types.h"
#include "myrand.h"
#include "graphics.h"

extern UBYTE playfield[PLAYFIELD_SIZE];
extern metasprite_t preview_items[PREVIEW_SIZE][3];

extern myrand_state_t r7, r81;

extern UBYTE playfield_anim;
extern const UBYTE animation[];

extern UBYTE random_put_scores;
extern UBYTE preview_placement;

void playfield_draw();
void playfield_draw_item(UBYTE x, UBYTE y, UBYTE color);

inline UBYTE playfield_get(UBYTE x, UBYTE y) {
    return playfield[(y * PLAYFIELD_WIDTH) + x] & PREVIEW_MASK;
}
inline void playfield_set(UBYTE x, UBYTE y, UBYTE color) {
    playfield[(y * PLAYFIELD_WIDTH) + x] = color;
}

UBYTE playfield_put_item(UWORD idx, UBYTE color);

void playfield_refresh_preview();
void playfield_randomize_preview();

void playfield_process_animation(UBYTE anim);

UBYTE playfield_put_random(UBYTE count, UBYTE color);
UBYTE playfield_put_previewed();

#endif