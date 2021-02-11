#include <gb/gb.h>

#include "scroller.h"
#include "myrand.h"
#include "score.h"
#include "graphics.h"

extern game_state_e game_state;

extern const UBYTE animation[];

extern myrand_state_t r7;

UBYTE scroll_pos_x = 0, scroll_pos_y = 0;
const unsigned char scroll_text[] = 
"COLOR LINES! A SMALL PUZZLE GAME FOR THE GAME BOY. "\
"MATCH FIVE OR MORE PIECES OF THE SAME SHAPE AND COLOR IN A LINE HORIZONTALLY, VERTICALLY OR DIAGONALLY TO SCORE POINTS. "\
"FIRST SELECT A PIECE TO MOVE, THEN CHOOSE WHERE TO MOVE IT. WATCH OUT THOUGH, IF THERE'S NO CLEAR PATH YOU WON'T BE ABLE TO REACH YOUR DESTINATION. "\
"EACH TURN, IF YOU FAILED TO MAKE A MATCH, THREE MORE PIECES WILL BE ADDED TO THE BOARD RANDOMLY. "\
"TRY TO GET THE HIGHEST SCORE BEFORE THE BOARD FILLS UP! "\
"CONTROLS: [D-PAD] - MOVE CURSOR; [A] - SELECT ITEM/MOVE SELECTED ITEM; [B] - PASS MOVE; "\
"[START] - SHOW SCORE; [SELECT] - TOGGLE MUSIC AND SOUND. "\
"THIS GAME WAS ORIGINALLY MADE FOR IBM PC BY OLGA DEMINA IN 1992. "\
"GAME BOY REMAKE BY TOXA. MUSIC AND SFX BY KABCORP. "\
"DEVELOPED USING ZALO'S GBDK-2020, \"EMULICIOUS\" GAME BOY EMULATOR AND VS.CODE DEBUG ADAPTER EXTENSION BY CALINDRO, "\
"MUSIC WAS COMPOSED USING HUGETRACKER, WRITTEN BY SUPERDISK. "\
"THANKS TO BBBBBR AND CHRIS MALTBY FOR TESTING AND USEFUL SUGGESTIONS. "\
"GREETINGS TO #GBDK DISCORD SERVER USERS AND EVERYONE I KNOW!         "\
"PRESS [START] TO BEGIN GAME.                    ";

const UBYTE * scroll_text_ptr = scroll_text;
void scroll_update_isr() {
    switch (LYC_REG) {
        case 0:
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
}

void scroll_reset() {
    scroll_pos_x = scroll_pos_y = 0; scroll_text_ptr = scroll_text;    
}

void scroll_set_pos(UBYTE x) {
    scroll_pos_x = x;
}

void scroll_process() {
    if ((scroll_pos_x & 0x07) == 0) {
        if (*scroll_text_ptr == 0) scroll_text_ptr = scroll_text;
        set_attributed_bkg_tile_xy(((scroll_pos_x >> 3) + 20) & 0x1f, 16, ascii_to_tile(*scroll_text_ptr), myrand(&r7) + 1);
        scroll_text_ptr++;
    }
    scroll_pos_x++; scroll_pos_y = animation[(scroll_pos_x >> 1) & ANIM_MASK];
}
