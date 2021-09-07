#include "score.h"
#include "graphics.h"

UBYTE score_text[16] = {
    ASCII_TO_TILE('B'), ASCII_TO_TILE('E'), ASCII_TO_TILE('S'), ASCII_TO_TILE('T'), ASCII_TO_TILE(':'), 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

UWORD score, old_score; 
UWORD highscore = 0;
UBYTE score_len;

#if defined(NINTENDO)
sram_record_t __at(0xA000) sram_highscore;
#elif defined(SEGA)
sram_record_t __at(0x8000) sram_highscore;
#endif
UBYTE score_anim = SCORE_ANIM_SIZE - 1;

metasprite_t score_display[SCORE_SIZE + 1];

inline UBYTE calc_crc(UWORD crc) {
    return 0x55 ^ (UBYTE)crc ^ (UBYTE)(crc >> 8);
}

UWORD score_load() {
    SWITCH_RAM(0);
    if ((sram_highscore.signature != HIGHSCORE_SIGNATURE) || 
       (sram_highscore.crc != calc_crc(sram_highscore.highscore)))  {
        sram_highscore.signature = HIGHSCORE_SIGNATURE; 
        sram_highscore.highscore = 0;
        sram_highscore.crc = calc_crc(0);
    }
    return sram_highscore.highscore;
}

UWORD score_save(UWORD score) {
    sram_highscore.signature = HIGHSCORE_SIGNATURE;
    sram_highscore.crc = calc_crc(score);
    return sram_highscore.highscore = score;
}

UBYTE score_add(UBYTE addend) {
    if (!addend) return FALSE;
    // add score
    if (addend > 8) {
        score += addend * 3; 
    } else if (addend > 6) {
        score += addend * 2; 
    } else {
        score += addend;
    }
    return TRUE;
}
