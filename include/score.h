#ifndef _SCORE_H_INCLUDE
#define _SCORE_H_INCLUDE

#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#include <game_types.h> 

extern UBYTE score_len;
extern UBYTE score_anim;

extern UWORD score, old_score, highscore; 

extern metasprite_t score_display[SCORE_SIZE + 1];
extern UBYTE score_text[16];

#if defined(NINTENDO)
#if defined(REFLASH)
extern sram_record_t sram_highscore;
#else
extern sram_record_t AT(0xA000) sram_highscore;
#endif
#elif defined(SEGA)
#undef REFLASH
extern sram_record_t AT(0x8000) sram_highscore;
#endif

UWORD score_load();
UWORD score_save(UWORD score);
UBYTE score_add(UBYTE addend);

#endif