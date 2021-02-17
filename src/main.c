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


void clear_viewport() {
    wait_vbl_done();
    HIDE_SPRITES; HIDE_BKG;
    OAM_item_t * ptr = shadow_OAM;
    for (UBYTE i = 0; i != 40; i++) ptr->y = 0, ptr++;    
    clear_screen();
    scroll_reset();
    SHOW_SPRITES; SHOW_BKG;
}

game_state_e intro_run() {
    // clear screen;
    clear_viewport();
    // restore zero sprite palette
    if (_cpu == CGB_TYPE) set_sprite_palette(0, 1, sprite_palettes);

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
    clear_viewport();
    // restore zero sprite palette
    if (_cpu == CGB_TYPE) set_sprite_palette(0, 1, sprite_palettes);

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

static UBYTE cursor_x, cursor_y;
static UBYTE selected, selected_x, selected_y;
static UBYTE anim, anim_curs;

inline void animation_next_step() {
    anim++; anim &= ANIM_MASK;
    if ((anim & 0x03) == 0) {
        anim_curs++; if (anim_curs == 7) anim_curs = 0;
    }
}

inline void actors_animate(UBYTE anim, UBYTE mx, UBYTE my) {
    // draw selection and cursor
    if (_cpu == CGB_TYPE) set_sprite_palette(0, 1, &sprite_palettes[(anim_curs + 1) << 2]);
    move_metasprite(cursor, 0x1c + (anim_curs << 2), 0, (cursor_x << 4) + 8, (cursor_y << 4) + 16);
    if (selected) {
        if (anim == 0) SOUND_JUMP;
        item[1].props = item[0].props = selected;
        move_metasprite(item, (selected - 1) << 2, 2, mx, my);
    } else {
        hide_metasprite(item, 2);
    }
}

game_state_e game_run() {
    static UBYTE joy, joy_old;

    // reset score
    score = 0; old_score = 1;

    // init random generators
    randomize();
    myrand_init(7, &r7);
    myrand_init(81, &r81);

    // clear playfield
    memset(playfield, 0, sizeof(playfield));
    
    // clear screen
    clear_viewport();

    // draw playfield
    playfield_draw();

    // put initial items
    playfield_put_random(5, 0);

    // next random items
    playfield_randomize_preview();

    cursor_x = cursor_y = 0;
    selected_x = selected_y = 0; selected = 0;
    anim_curs = anim = 0;

    joy_old = joy = 0;
    // main game loop
    while (TRUE) {
        // process input
        joy = joypad();

        if (joy != joy_old) {
            switch (joy) {
                case J_LEFT:
                    if (cursor_x) cursor_x--; else cursor_x = PLAYFIELD_WIDTH - 1;
                    if ((!selected) && ((sound_control & MUSIC_ON) == 0)) SOUND_CURSOR;
                    break;
                case J_RIGHT:
                    if (cursor_x < (PLAYFIELD_WIDTH - 1)) cursor_x++; else cursor_x = 0;
                    if ((!selected) && ((sound_control & MUSIC_ON) == 0)) SOUND_CURSOR;
                    break;
                case J_UP:
                    if (cursor_y) cursor_y--; else cursor_y = PLAYFIELD_HEIGHT - 1;
                    if ((!selected) && ((sound_control & MUSIC_ON) == 0)) SOUND_CURSOR;
                    break;
                case J_DOWN:
                    if (cursor_y < (PLAYFIELD_HEIGHT - 1)) cursor_y++; else cursor_y = 0;
                    if ((!selected) && ((sound_control & MUSIC_ON) == 0)) SOUND_CURSOR;
                    break;
                case J_A: {
                    if (selected) {
                        if (playfield_get(cursor_x, cursor_y) == 0) {
                            path_length = lee_find_path(selected_x, selected_y, cursor_x, cursor_y);
                            if (path_length != LEE_MAX_STEPS) {       
                                lee_restore_path(cursor_x, cursor_y, path);

                                // animate path 
                                UBYTE mx = (selected_x << 4) + 8u;
                                UBYTE my = (selected_y << 4) + 16u;
                                for (UBYTE i = 0; i <= path_length; i++) {
                                    UBYTE cx = (lee_get_coords_x(path[i]) << 4u) + 8u;
                                    UBYTE cy = (lee_get_coords_y(path[i]) << 4u) + 16u;
                                    while (TRUE) {
                                        if (mx < cx) mx++; else 
                                        if (mx > cx) mx--; else 
                                        if (my < cy) my++; else 
                                        if (my > cy) my--; else break;

                                        // next animation step
                                        animation_next_step();
                                        // animate sprites
                                        actors_animate(anim, mx, my - animation[anim]);
                                        // animate playfield
                                        playfield_process_animation(anim);

                                        wait_vbl_done(); 
                                    }
                                }
                                // set new values
                                playfield_set(selected_x, selected_y, 0);
                                playfield_draw_item(selected_x, selected_y, 0);
                                UBYTE tmp_score = playfield_put_item((cursor_y * PLAYFIELD_WIDTH) + cursor_x, selected);
                                if (!tmp_score) {
                                    if (!playfield_put_previewed()) return game_over;
                                    playfield_randomize_preview();
                                } else {
                                    score_add(tmp_score);
                                    playfield_refresh_preview();
                                }
                                selected = FALSE;
                            }
                        } else {
                            UBYTE selected_new = playfield_get(cursor_x, cursor_y);
                            if (selected_new) {
                                playfield_draw_item(selected_x, selected_y, selected);
                                selected_x = cursor_x, selected_y = cursor_y;
                                selected = selected_new;
                                playfield_draw_item(selected_x, selected_y, 0);
                            }
                        }
                    } else {
                        selected = playfield_get(cursor_x, cursor_y);
                        if (selected) {
                            selected_x = cursor_x, selected_y = cursor_y;
                            playfield_draw_item(selected_x, selected_y, 0);
                        }
                    } 
                    break;
                }
                case J_B:
                    if (!playfield_put_previewed()) return game_over;
                    playfield_randomize_preview();
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
        animation_next_step();
        // animate sprites
        actors_animate(anim, (selected_x << 4) + 8, (selected_y << 4) + 16 - animation[anim]);
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
        TMA_REG = 0xC0u; TAC_REG = 0x07u;
        add_TIM(music_update);
        LYC_REG = 0; STAT_REG |= 0b01000000;
        add_LCD(scroll_update_isr);
        set_interrupts(VBL_IFLAG | TIM_IFLAG | LCD_IFLAG);
    }
    toggle_sound_settings(0);

    OBP0_REG = 0x3bu; BGP_REG = 0x1Bu;
    if (_cpu == CGB_TYPE) {
        set_bkg_palette(0, 8, background_palettes);
        set_sprite_palette(0, 8, sprite_palettes);
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