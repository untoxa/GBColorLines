#include "score.h"
#include "graphics.h"
#include "sound.h"

UBYTE score_text[16] = {
    ASCII_TO_TILE('B'), ASCII_TO_TILE('E'), ASCII_TO_TILE('S'), ASCII_TO_TILE('T'), ASCII_TO_TILE(':'), 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

UWORD score, old_score; 
UWORD highscore = 0;
UBYTE score_len;

sram_record_t __at(0xA000) sram_highscore;

UBYTE score_anim = SCORE_ANIM_SIZE - 1;

metasprite_t score_display[SCORE_SIZE + 1];

void score_add(UBYTE addend) {
    if (!addend) return;
    // play sound
    SOUND_CLEAR;
    // add score
    if (addend > 8) {
        score += addend * 3; 
    } else if (addend > 6) {
        score += addend * 2; 
    } else {
        score += addend;
    }
}
