#ifndef _SCORE_H_INCLUDE
#define _SCORE_H_INCLUDE

#include <gb/gb.h>
#include <gb/metasprites.h>

#include <game_types.h> 

extern UBYTE score_len;
extern UBYTE score_anim;

extern UWORD score, old_score, highscore; 

extern metasprite_t score_display[SCORE_SIZE + 1];
extern UBYTE score_text[16];

extern sram_record_t __at(0xA000) sram_highscore;

void score_add(UBYTE addend);

#endif