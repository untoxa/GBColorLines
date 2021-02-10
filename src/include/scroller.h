#ifndef _SCROLLER_H_INCLUDE
#define _SCROLLER_H_INCLUDE

#include "game_types.h"

#define SCROLL_Y_POS (15 * 8) - 1 

inline UBYTE ascii_to_tile(UBYTE ch) {
    return (ch > 0x20) ? ((ch - 0x21) << 1) + 0x80 : 0;
}

void scroll_update_isr();

void scroll_reset();
void scroll_set_pos(UBYTE x);
void scroll_process();

#endif