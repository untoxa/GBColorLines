#include <gbdk/platform.h>

#include <rand.h>

#include "scroller.h"
#include "score.h"
#include "graphics.h"

extern game_state_e game_state;

extern const UBYTE animation[];

#ifdef CATSKULL_LOGO
#define PUBLISHER_TEXT "PUBLISHED BY CATSKULL GAMES. "
#else
#define PUBLISHER_TEXT ""
#endif


UBYTE scroll_pos_x = 0, scroll_pos_y = 0;
const unsigned char scroll_text[] =
"COLOR LINES! A SMALL CROSS-PLATFORM PUZZLE GAME FOR THE NINTENDO GAME BOY, SEGA GAME GEAR AND SEGA MASTER SYSTEM. "\
"MATCH FIVE OR MORE PIECES OF THE SAME SHAPE AND COLOR IN A LINE HORIZONTALLY, VERTICALLY OR DIAGONALLY TO SCORE POINTS. "\
"FIRST SELECT A PIECE TO MOVE, THEN CHOOSE WHERE TO MOVE IT. WATCH OUT THOUGH, IF THERE'S NO CLEAR PATH YOU WON'T BE ABLE TO REACH YOUR DESTINATION. "\
"EACH TURN, IF YOU FAILED TO MAKE A MATCH, THREE MORE PIECES WILL BE ADDED TO THE BOARD RANDOMLY. "\
"TRY TO GET THE HIGHEST SCORE BEFORE THE BOARD FILLS UP! "\
"CONTROLS: [D-PAD] - MOVE CURSOR; [1]/[A] - SELECT ITEM/MOVE SELECTED ITEM; [2]/[B] - PASS MOVE; "\
"[START] - SHOW SCORE; [SELECT] - TOGGLE MUSIC AND SOUND. "\
"THIS GAME WAS ORIGINALLY MADE FOR IBM PC BY OLEG DEMIN IN 1992. "\
"GAME BOY REMAKE BY TOXA. MUSIC AND SFX BY KABCORP. "\
PUBLISHER_TEXT\
"DEVELOPED USING ZALO'S GBDK-2020, \"EMULICIOUS\" GAME BOY EMULATOR AND VS.CODE DEBUG ADAPTER EXTENSION BY CALINDRO, "\
"MUSIC WAS COMPOSED USING HUGETRACKER, WRITTEN BY SUPERDISK. "\
"THANKS TO BBBBBR AND CHRIS MALTBY FOR TESTING AND USEFUL SUGGESTIONS. "\
"GREETINGS TO #GBDK DISCORD SERVER USERS AND EVERYONE I KNOW!         "\
"PRESS [1]/[START] TO BEGIN GAME.                    ";

const UBYTE * scroll_text_ptr = scroll_text;
void scroll_update_isr(void) {
#if defined(NINTENDO)
    switch (LYC_REG) {
        case 0:
            SCX_REG = (game_state == game_intro) ? 4 : 0;
            SCY_REG = 0;
            LYC_REG = 39;
            break;
        case 39:
            SCX_REG = 0; SCY_REG = 0;
            LYC_REG = SCROLL_Y_POS;
            break;
        case SCROLL_Y_POS:
            SCX_REG = scroll_pos_x; SCY_REG = scroll_pos_y;
            LYC_REG = (UBYTE)(SCROLL_Y_POS + 16);
            break;
        case SCROLL_Y_POS + 16:
            SCX_REG = 0;
            SCY_REG = ((game_state == game_intro) && (highscore)) ? 8 : 0;
            LYC_REG = 0;
            break;
    }
#endif
}

void scroll_reset(void) {
    scroll_pos_x = scroll_pos_y = 0; scroll_text_ptr = scroll_text;
}

void scroll_set_pos(UBYTE x) {
    scroll_pos_x = x;
}

UBYTE scroll_get_pos(void) {
    return scroll_pos_x;
}


void scroll_process(void) {
    if ((scroll_pos_x & 0x07) == 0) {
        if (*scroll_text_ptr == 0) scroll_text_ptr = scroll_text;
        set_attributed_bkg_tile_xy(((scroll_pos_x >> 3) + DEVICE_SCREEN_WIDTH) & 0x1f, 16, ascii_to_tile(*scroll_text_ptr), (rand() % 7) + 1);
        scroll_text_ptr++;
    }
    scroll_pos_x++; scroll_pos_y = animation[(scroll_pos_x >> 1) & ANIM_MASK];
#if defined(SEGA)
    while (VCOUNTER != (SCROLL_Y_POS + (DEVICE_SCREEN_Y_OFFSET * 8)));
    __WRITE_VDP_REG(VDP_RSCX, -scroll_pos_x);
    while (VCOUNTER != (SCROLL_Y_POS + (DEVICE_SCREEN_Y_OFFSET * 8) + 16));
    __WRITE_VDP_REG(VDP_RSCX, 0);
#endif
}
