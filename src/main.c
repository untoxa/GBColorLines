#include <gb/gb.h>
#include <gb/metasprites.h>
#include <gb/gbdecompress.h>

#include <string.h>
#include <stdlib.h>

#include "game_types.h"
#include "scroller.h"
#include "sound.h"
#include "graphics.h"
#include "myrand.h"
#include "lee.h"

UBYTE playfield[PLAYFIELD_SIZE];

myrand_state_t r7, r81;

metasprite_t ball[3];

#define PREVIEW_SIZE 3
UBYTE preview_colors[PREVIEW_SIZE];
metasprite_t preview_balls[PREVIEW_SIZE][3];

const UBYTE animation[] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 2, 2, 1, 1};

UBYTE playfield_anim = 0;

UBYTE score_text[16] = {
    ASCII_TO_TILE('B'), ASCII_TO_TILE('E'), ASCII_TO_TILE('S'), ASCII_TO_TILE('T'), ASCII_TO_TILE(':'), 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

UWORD score, old_score, score_len;
UWORD highscore = 0;

sram_record_t __at(0xA000) sram_highscore;

UBYTE score_anim = SCORE_ANIM_SIZE - 1;

#define SCORE_SIZE 5
metasprite_t score_display[SCORE_SIZE + 1];

game_state_e game_state = game_intro;


UBYTE test_collision(UBYTE x, UBYTE y) {
    if (x > (PLAYFIELD_WIDTH - 1)) return FALSE;
    if (y > (PLAYFIELD_HEIGHT - 1)) return FALSE;
    return (playfield[(y * PLAYFIELD_WIDTH) + x] == 0);
}

UWORD path[LEE_MAX_STEPS];
UBYTE path_length;

#define SOUND_ON 0x01
#define MUSIC_ON 0x02
UBYTE sound_control = SOUND_ON | MUSIC_ON;

void wait_pad_up() {
    if (joypad()) {
        do {
            wait_vbl_done();        
        } while (joypad());
    }
}

void clear_screen() {
    if (_cpu == CGB_TYPE) {
        VBK_REG = 1;
        fill_bkg_rect(0, 0, 22, 20, 0);
        VBK_REG = 0;
    }
    fill_bkg_rect(0, 0, 22, 20, 0);
}


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

void playfield_draw() {
    for (UBYTE i = 0; i < (PLAYFIELD_HEIGHT * 2); i+=2) {
        set_bkg_tiles(0, i, 18, 2, field_row);
    }
}

void playfield_draw_item(UBYTE x, UBYTE y, UBYTE color) {
    UBYTE attr = color & 0x07u;
    UBYTE attributes[4] = {attr, attr, attr, attr};
    UBYTE tiles[4] = {(attr << 2) + 0x01, (attr << 2) + 0x03, (attr << 2) + 0x02, (attr << 2) + 0x04};

    if (_cpu == CGB_TYPE) {
        VBK_REG = 1;
        set_bkg_tiles(x << 1, y << 1, 2, 2, attributes);    
        VBK_REG = 0;
    }
    set_bkg_tiles(x << 1, y << 1, 2, 2, tiles);    
}

inline UBYTE playfield_get(UBYTE x, UBYTE y) {
    return playfield[(y * PLAYFIELD_WIDTH) + x];
}
inline void playfield_set(UBYTE x, UBYTE y, UBYTE color) {
    playfield[(y * PLAYFIELD_WIDTH) + x] = color;
}

typedef struct {
    UBYTE x, y, count;
} lineprop_t;

UBYTE playfield_put_item(UWORD idx, UBYTE color) {
    UBYTE x = idx % PLAYFIELD_WIDTH, y = idx / PLAYFIELD_HEIGHT;

    // draw
    playfield_draw_item(x, y, color);
    playfield[idx] = color;

    UBYTE score = 0;

    // check disappear
    lineprop_t h = {x, y, 1}, v = {x, y, 1}, d1 = {x, y, 1}, d2 = {x, y, 1};
    UBYTE dx, dy;

    // check lines
    // check hline
    while ((h.x) && (playfield_get(h.x - 1, h.y) == color)) h.x--;
    dx = h.x;
    while ((dx < (PLAYFIELD_WIDTH - 1)) && (playfield_get(dx + 1, h.y) == color)) dx++, h.count++;
    // check vline
    while ((v.y) && (playfield_get(v.x, v.y - 1) == color)) v.y--;
    dy = v.y;
    while ((dy < (PLAYFIELD_HEIGHT - 1)) && (playfield_get(v.x, dy + 1) == color)) dy++, v.count++;
    // cleck d1
    while ((d1.x) && (d1.y) && (playfield_get(d1.x - 1, d1.y - 1) == color)) d1.x--, d1.y--;
    dx = d1.x, dy = d1.y;
    while ((dx < (PLAYFIELD_WIDTH - 1)) && (dy < (PLAYFIELD_HEIGHT - 1)) && (playfield_get(dx + 1, dy + 1) == color)) dx++, dy++, d1.count++;
    // cleck d2
    while ((d2.x) && (d2.y < (PLAYFIELD_HEIGHT - 1)) && (playfield_get(d2.x - 1, d2.y + 1) == color)) d2.x--, d2.y++;
    dx = d2.x, dy = d2.y;
    while ((dx < (PLAYFIELD_WIDTH - 1)) && (dy) && (playfield_get(dx + 1, dy - 1) == color)) dx++, dy--, d2.count++;

    // disappear lines:
    // disappear hline
    if (h.count >= 5) {
        score += h.count;
        while (h.count) playfield_set(h.x, h.y, 0), playfield_draw_item(h.x, h.y, 0), h.count--, h.x++;
    }
    // disappear vline
    if (v.count >= 5) {
        score += v.count;
        while (v.count) playfield_set(v.x, v.y, 0), playfield_draw_item(v.x, v.y, 0), v.count--, v.y++;
    }
    // disappear d1
    if (d1.count >= 5) {
        score += d1.count;
        while (d1.count) playfield_set(d1.x, d1.y, 0), playfield_draw_item(d1.x, d1.y, 0), d1.count--, d1.x++, d1.y++;
    }
    // disappear d2
    if (d2.count >= 5) {
        score += d2.count;
        while (d2.count) playfield_set(d2.x, d2.y, 0), playfield_draw_item(d2.x, d2.y, 0), d2.count--, d2.x++, d2.y--;
    }

    return score;
}

void playfield_randomize_preview() {
    for (UBYTE i = 0; i != PREVIEW_SIZE; i++) {
        preview_balls[i][1].props = preview_balls[i][0].props = preview_colors[i] = myrand(&r7) + 1; 
    }
}

void playfield_process_animation(UBYTE anim) {
    if (anim & 1) {
        playfield_anim++; playfield_anim &= ANIM_MASK;
    }
    for (UBYTE i = 0; i != PREVIEW_SIZE; i++) {
        move_metasprite(preview_balls[i], (preview_colors[i] - 1) << 2, (i << 1) + 4, 151 + animation[(playfield_anim + (i << 1)) & ANIM_MASK], 64 + (i << 4));
    }
    if (score != old_score) {
        old_score = score;
        UBYTE * pc = score_text + 5;
        score_len = strlen(utoa(score, pc));
        for (UBYTE i = 0; i != SCORE_SIZE; i++, pc++) {
            score_display[i].dtile = (*pc) ? ((*pc - '0') << 1) + 0x9e : 0xfc;        
        }
        score_anim = 0;
    } 

    if (score_anim < SCORE_ANIM_SIZE) {
        move_metasprite(score_display, 0, 16, 160 - (score_len << 3), score_animation[score_anim]);
        score_anim++;
    }
}

UBYTE playfield_put_random(UBYTE count) {
    for (UBYTE i = 0; i != count; i++) {
        UBYTE r = myrand(&r81) + 1;
        UBYTE exit = FALSE;
        UBYTE * pf = playfield;
        while (r) {
            if (*pf == 0) {
                if (--r == 0) break;
                exit = FALSE;
            }
            if (++pf == (playfield + sizeof(playfield))) {
                if (exit) return FALSE;
                pf = playfield;
                exit = TRUE;
            }
        }
        if (i < PREVIEW_SIZE) {
            // get colors from preview
            score_add(playfield_put_item((UWORD)(pf - playfield), preview_balls[i][0].props));       
        } else {
            // generate random
            score_add(playfield_put_item((UWORD)(pf - playfield), myrand(&r7) + 1));       
        }
    }

    // next colors
    playfield_randomize_preview();

    // pop sound
    SOUND_POP;

    // check for free space
    for (UBYTE i = 0; i < PLAYFIELD_SIZE; i++)
        if (playfield[i] == 0) return TRUE;

    return FALSE;
}

void toggle_sound_settings(UBYTE addend) {
    sound_control += addend; sound_control &= (SOUND_ON | MUSIC_ON); 
    if (sound_control & MUSIC_ON) music_play(); else music_stop();
    sound_playing = (sound_control & SOUND_ON);
}


game_state_e intro_run() {
    // clear screen;
    wait_vbl_done();    
    clear_screen();

    scroll_reset();

    // remove all sprites from screen
    memset(shadow_OAM, 0, 40 * sizeof(OAM_item_t));

    // display some background
    if (_cpu == CGB_TYPE) {
        VBK_REG = 1;
        set_bkg_tiles(2, 5, 15, 3, intro_attr);
        VBK_REG = 0;
    }
    set_bkg_tiles(2, 5, 15, 3, intro_map);

    UBYTE start_sprite = move_metasprite(start_msg, 0, 0, 42, 96);

    if (highscore) {
        UBYTE *pc = score_text + 5;
        UBYTE len = strlen(utoa(highscore, pc));
        while (*pc) {
            *pc++ = ((*pc - 0x21) << 1) + 0x80;
        }
        set_bkg_tiles(15 - len, 18, 5 + len, 1, score_text); 
    }

    // loop until start
    UBYTE wait = 0;
    while (TRUE) {
        switch (joypad()) {
            case J_START: 
                wait_pad_up();
                return game_play;
            case J_SELECT:
                if (wait) break;
                toggle_sound_settings(2);
                wait = 10;
                break;
        }
        // animate screen
        if (sys_time & 1) {
            playfield_anim++; playfield_anim &= ANIM_MASK;
            move_metasprite(start_msg, 0, 0, 42 + animation[playfield_anim], 96);
            UBYTE base = start_sprite;
            for (UBYTE j = 0; j != TITLE_SIZE; j++) {
                base += move_metasprite(title[j], 0, base, (j << 4) + 52, 56 + (animation[(playfield_anim + (j << 1)) & ANIM_MASK] << 1));
            }
        }
        // process delay
        if (wait) wait--;

        wait_vbl_done();
        // process scroll
        scroll_process();
    }
}


game_state_e over_run() {
    // clear screen;
    wait_vbl_done();    
    scroll_reset();
    clear_screen();

    // remove all sprites from screen
    memset(shadow_OAM, 0, 40 * sizeof(OAM_item_t));

    // display some background
    if (_cpu == CGB_TYPE) {
        VBK_REG = 1;
        set_bkg_tiles(2, 5, 15, 3, intro_attr);
        VBK_REG = 0;
    }
    set_bkg_tiles(2, 5, 15, 3, intro_map);

    UBYTE tmp_score_text[16];
    memcpy(tmp_score_text, "SCORE:", 6);
    UBYTE *pc = tmp_score_text;
    UBYTE len = strlen(utoa(score, tmp_score_text + 6)) + 6;
    while (*pc) {
        *pc++ = ((*pc - 0x21) << 1) + 0x80;
    }
    set_bkg_tiles(((20 - len) >> 1) + 1, 15, len, 1, tmp_score_text); 
    scroll_set_pos((len & 1) ? 4 : 8);

    // wait pad is up
    wait_pad_up();

    UBYTE wait = 0;
    while (TRUE) {
        switch (joypad()) {
            case 0:
                break;
            case J_SELECT:
                if (wait) break;
                toggle_sound_settings(2);
                wait = 10;
                break;
            default: 
                wait_pad_up();
                return game_intro;
        }
        // process delay
        if (wait) wait--;

        // animate screen
        if (sys_time & 1) {
            playfield_anim++; playfield_anim &= ANIM_MASK;
            UBYTE base = 0;
            for (UBYTE j = 0; j != OVER_SIZE; j++) {
                base += move_metasprite(over[j], 0, base, (j << 4) + 60, 56 + (animation[(playfield_anim + (j << 1)) & ANIM_MASK] << 1));
            }
        }
        wait_vbl_done();
    }
}


game_state_e game_run() {
    static UBYTE x, y;
    static UBYTE selected, sx, sy, anim, anim_curs;
    static UBYTE joy, joy_old;

    // reset score
    score = 0; old_score = 1;

    // remove all sprites from screen
    memset(shadow_OAM, 0, 40 * sizeof(OAM_item_t));

    // init random generators
    randomize();
    myrand_init(7, &r7);
    myrand_init(81, &r81);

    // clear playfield
    memset(playfield, 0, sizeof(playfield));
    
    // clear screen
    wait_vbl_done();
    scroll_reset();
    clear_screen();

    // draw playfield
    playfield_draw();

    // put initial items
    playfield_randomize_preview();
    playfield_put_random(5);

    x = y = 0;
    sx = sy = 0; selected = 0;
    anim_curs = anim = 0;

    joy_old = joy = 0;
    // main game loop
    while (TRUE) {
        // process input
        joy = joypad();

        if (joy != joy_old) {
            switch (joy) {
                case J_LEFT:
                    if (x) x--; else x = PLAYFIELD_WIDTH - 1;
                    if ((!selected) && ((sound_control & MUSIC_ON) == 0)) SOUND_CURSOR;
                    break;
                case J_RIGHT:
                    if (x < (PLAYFIELD_WIDTH - 1)) x++; else x = 0;
                    if ((!selected) && ((sound_control & MUSIC_ON) == 0)) SOUND_CURSOR;
                    break;
                case J_UP:
                    if (y) y--; else y = PLAYFIELD_HEIGHT - 1;
                    if ((!selected) && ((sound_control & MUSIC_ON) == 0)) SOUND_CURSOR;
                    break;
                case J_DOWN:
                    if (y < (PLAYFIELD_HEIGHT - 1)) y++; else y = 0;
                    if ((!selected) && ((sound_control & MUSIC_ON) == 0)) SOUND_CURSOR;
                    break;
                case J_A: {
                    if (selected) {
                        if (playfield_get(x, y) == 0) {
                            path_length = find_path(sx, sy, x, y);
                            if (path_length != LEE_MAX_STEPS) {       
                                restore_path(x, y, path);

                                // animate path 
                                UBYTE mx = (sx << 4) + 8u;
                                UBYTE my = (sy << 4) + 16u;
                                for (UBYTE i = 0; i <= path_length; i++) {
                                    UBYTE cx = ((UBYTE)(path[i] & LEE_MASK) << 4u) + 8u;
                                    UBYTE cy = ((UBYTE)(path[i] >> LEE_BITS) << 4u) + 16u;
                                    while (TRUE) {
                                        if (mx < cx) mx++; else 
                                        if (mx > cx) mx--; else 
                                        if (my < cy) my++; else 
                                        if (my > cy) my--; else break;

                                        anim++; anim &= ANIM_MASK;

                                        if (anim == 0) SOUND_BALL;

                                        move_metasprite(ball, (selected - 1) << 2, 2, mx, my - animation[anim]);
                                        playfield_process_animation(anim);

                                        wait_vbl_done(); 
                                    }
                                }
                                // set new values
                                playfield_set(sx, sy, 0);
                                playfield_draw_item(sx, sy, 0);
                                UBYTE tmp_score = playfield_put_item((y * PLAYFIELD_WIDTH) + x, selected);
                                if (!tmp_score) {
                                    if (!playfield_put_random(3)) return game_over;
                                } else {
                                    score_add(tmp_score);
                                }
                                selected = 0;
                            }
                        } else {
                            UBYTE selected_new = playfield_get(x, y);
                            if (selected_new) {
                                playfield_draw_item(sx, sy, selected);
                                sx = x, sy = y;
                                selected = selected_new;
                                playfield_draw_item(sx, sy, 0);
                            }
                        }
                    } else {
                        selected = playfield_get(x, y);
                        if (selected) {
                            sx = x, sy = y;
                            playfield_draw_item(sx, sy, 0);
                        }
                    } 
                    break;
                }
                case J_B:
                    if (!playfield_put_random(3)) return game_over;
                    break;
                case J_START:
                    if (score_anim == SCORE_ANIM_SIZE) score_anim = 0;
                    break;
                case J_SELECT:
                    toggle_sound_settings(1);
                    break;
            }
            joy_old = joy;
        }

        // next animation step
        anim++; anim &= ANIM_MASK;

        if ((anim & 0x03) == 0) {
            anim_curs++; if (anim_curs == 7) anim_curs = 0;
        }

        // draw selection and cursor
        move_metasprite(cursor, 0x1c + (anim_curs << 2), 0, (x << 4) + 8, (y << 4) + 16);
        if (selected) {
            if (anim == 0) SOUND_BALL;
            ball[1].props = ball[0].props = selected;
            move_metasprite(ball, (selected - 1) << 2, 2, (sx << 4) + 8, (sy << 4) + 16 - animation[anim]);
        } else {
            hide_metasprite(ball, 2);
        }
        // animate playfield
        playfield_process_animation(anim);

        // wait vblank
        wait_vbl_done();
    }
}


void main() {
    NR52_REG = 0x80u;
    NR51_REG = 0xffu;
    NR50_REG = 0x77u;

    __critical {
	    TMA_REG=0x00U; TAC_REG=0b00000111;
        add_TIM(music_update);
        LYC_REG = 0; STAT_REG |= 0b01000000;
        add_LCD(scroll_update_isr);
        set_interrupts(VBL_IFLAG | TIM_IFLAG | LCD_IFLAG);
    }
    toggle_sound_settings(0);

    OBP0_REG = BGP_REG = 0x1Bu;
    if (_cpu == CGB_TYPE) {
        set_bkg_palette(0, 8, palettes);
        set_sprite_palette(0, 8, palettes);
    }
    gb_decompress_bkg_data(0, bkg_tiles);
    gb_decompress_sprite_data(0, sprite_tiles);

    gb_decompress_bkg_data(0x80, font);

    memcpy(ball, ball_defaults, sizeof(ball));
    for (UBYTE i = 0; i != PREVIEW_SIZE; i++) 
        memcpy(preview_balls[i], ball_defaults, sizeof(preview_balls[0]));

    memcpy(score_display, score_display_defaults, sizeof(score_display));

    ENABLE_RAM_MBC5; SWITCH_RAM_MBC5(0);
    if (sram_highscore.signature != HIGHSCORE_SIGNATURE) {
        sram_highscore.signature = HIGHSCORE_SIGNATURE; sram_highscore.highscore = 0; 
    } else {
        highscore = sram_highscore.highscore;
    }

    SPRITES_8x16; SHOW_SPRITES; SHOW_BKG;

    randomize();
    myrand_init(7, &r7);

    while (TRUE) {

        switch (game_state) {
            case game_intro:
                game_state = intro_run();
                break;
            case game_play:
                game_state = game_run();
                // update highscore
                if (highscore < score) {
                    sram_highscore.signature = HIGHSCORE_SIGNATURE; 
                    sram_highscore.highscore = highscore = score;
                }
                break;
            case game_over:
                game_state = over_run();
                break;
        }
    }
}