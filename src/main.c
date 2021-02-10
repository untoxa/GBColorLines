#include <gb/gb.h>
#include <gb/metasprites.h>
#include <gb/gbdecompress.h>

#include <string.h>
#include <stdlib.h>

#include "game_types.h"
#include "score.h"
#include "scroller.h"
#include "sound.h"
#include "graphics.h"
#include "myrand.h"
#include "lee.h"
#include "playfield.h"

myrand_state_t r7, r81;

metasprite_t item[3];

const UBYTE animation[] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 2, 2, 1, 1};

game_state_e game_state = game_intro;


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

void toggle_sound_settings(UBYTE addend) {
    sound_control += addend; sound_control &= (SOUND_ON | MUSIC_ON); 
    if (sound_control & MUSIC_ON) music_play(); else music_stop();
    sound_playing = (sound_control & SOUND_ON);
}


UBYTE test_collision(UBYTE x, UBYTE y) {
    if (x > (PLAYFIELD_WIDTH - 1)) return FALSE;
    if (y > (PLAYFIELD_HEIGHT - 1)) return FALSE;
    return (playfield[(y * PLAYFIELD_WIDTH) + x] == 0);
}


game_state_e intro_run() {
    // clear screen;
    wait_vbl_done();    
    clear_screen();

    scroll_reset();

    // remove all sprites from screen
    memset(shadow_OAM, 0, 40 * sizeof(OAM_item_t));

    // display some background
    set_attributed_bkg_tiles(2, 5, 15, 3, intro_map, intro_attr);

    UBYTE start_sprite = move_metasprite(start_msg, 0, 0, 42, 96);

    if (highscore) {
        UBYTE *pc = score_text + 5;
        UBYTE len = strlen(utoa(highscore, pc));
        while (*pc) {
            *pc++ = ((*pc - 0x21) << 1) + 0x80;
        }
        set_bkg_tiles_blank(15 - len, 18, 5 + len, 1, score_text); 
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
    set_attributed_bkg_tiles(2, 5, 15, 3, intro_map, intro_attr);

    UBYTE tmp_score_text[16];
    memcpy(tmp_score_text, "SCORE:", 6);
    UBYTE *pc = tmp_score_text;
    UBYTE len = strlen(utoa(score, tmp_score_text + 6)) + 6;
    while (*pc) {
        *pc++ = ((*pc - 0x21) << 1) + 0x80;
    }
    set_bkg_tiles_blank(((20 - len) >> 1) + 1, 15, len, 1, tmp_score_text); 
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

                                        if (anim == 0) SOUND_JUMP;

                                        move_metasprite(item, (selected - 1) << 2, 2, mx, my - animation[anim]);
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
            if (anim == 0) SOUND_JUMP;
            item[1].props = item[0].props = selected;
            move_metasprite(item, (selected - 1) << 2, 2, (sx << 4) + 8, (sy << 4) + 16 - animation[anim]);
        } else {
            hide_metasprite(item, 2);
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

    memcpy(item, item_defaults, sizeof(item));
    for (UBYTE i = 0; i != PREVIEW_SIZE; i++) 
        memcpy(preview_items[i], item_defaults, sizeof(preview_items[0]));

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