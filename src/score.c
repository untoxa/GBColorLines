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
#if defined(REFLASH)
#include "flasher.h"
sram_record_t sram_highscore;
#else
sram_record_t AT(0xA000) sram_highscore;
#endif
#elif defined(SEGA)
sram_record_t AT(0x8000) sram_highscore;
#endif
UBYTE score_anim = SCORE_ANIM_SIZE - 1;

metasprite_t score_display[SCORE_SIZE + 1];

inline UBYTE calc_crc(UWORD crc) {
    return 0x55 ^ (UBYTE)crc ^ (UBYTE)(crc >> 8);
}

UWORD score_load(void) {
#if defined(REFLASH)
    flash_restore_data((void *)&sram_highscore, sizeof(sram_highscore));
#else
    SWITCH_RAM(0);
#endif
    if ((sram_highscore.signature != HIGHSCORE_SIGNATURE) ||
       (sram_highscore.crc != calc_crc(sram_highscore.highscore)))  {
        sram_highscore.signature = HIGHSCORE_SIGNATURE;
        sram_highscore.highscore = 0;
        sram_highscore.crc = calc_crc(0);
        return 0;
    }
    return sram_highscore.highscore;
}

UWORD score_save(UWORD score) {
    sram_highscore.signature = HIGHSCORE_SIGNATURE;
    sram_highscore.crc = calc_crc(score);
    sram_highscore.highscore = score;
#if defined(REFLASH)
    flash_save_data((void *)&sram_highscore, sizeof(sram_highscore));
#endif
    return score;
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
