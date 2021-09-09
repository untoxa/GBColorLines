#include "game.h"

myrand_state_t r7, r81;

metasprite_t item[3];

const UBYTE animation[] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 2, 2, 1, 1};


UWORD path[LEE_MAX_STEPS];
UBYTE path_length;

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
#ifdef NINTENDO
    if (DEVICE_SUPPORTS_COLOR) set_sprite_palette(0, 1, &sprite_palettes[(anim_curs + 1) << 2]);
#endif
    move_metasprite(cursor, 
                    0x1c + (anim_curs << 2), 
                    0, 
                    (FIELD_OFFSET_X << 3) + DEVICE_SPRITE_PX_OFFSET_X + (cursor_x << 4), 
                    (FIELD_OFFSET_Y << 3) + DEVICE_SPRITE_PX_OFFSET_Y + (cursor_y << 4));
    if (selected) {
        if (anim == 0) SOUND_JUMP;
#ifdef NINTENDO
        item[1].props = item[0].props = selected;
#endif
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

    wait_vbl_done();
    SHOW_SPRITES; SHOW_BKG;

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
                                UBYTE mx = (selected_x << 4);
                                UBYTE my = (selected_y << 4);
                                for (UBYTE i = 0; i <= path_length; i++) {
                                    UBYTE cx = lee_get_coords_x(path[i]) << 4u;
                                    UBYTE cy = lee_get_coords_y(path[i]) << 4u;
                                    while (TRUE) {
                                        if (mx < cx) mx++; else 
                                        if (mx > cx) mx--; else 
                                        if (my < cy) my++; else 
                                        if (my > cy) my--; else break;

                                        // next animation step
                                        animation_next_step();
                                        // animate sprites
                                        actors_animate(anim, 
                                                       (FIELD_OFFSET_X << 3) + DEVICE_SPRITE_PX_OFFSET_X + mx, 
                                                       (FIELD_OFFSET_Y << 3) + DEVICE_SPRITE_PX_OFFSET_Y + my - animation[anim]);
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
                                    if (!playfield_put_previewed()) {
                                        wait_vbl_done();
                                        HIDE_SPRITES; HIDE_BKG;
                                        return game_over;
                                    }
                                    playfield_randomize_preview();
                                } else {
                                    if (score_add(tmp_score)) SOUND_CLEAR;
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
                    if (!playfield_put_previewed()) {
                        wait_vbl_done();
                        HIDE_SPRITES; HIDE_BKG;
                        return game_over;
                    }
                    playfield_randomize_preview();
                    break;
#ifndef MASTERSYSTEM
                case J_START:
                    if (score_anim == SCORE_ANIM_SIZE) score_anim = 0;
                    break;
#endif
#ifdef NINTENDO
                case J_SELECT:
                    toggle_sound_settings(1);
                    break;
#endif
            }
            joy_old = joy;
        }

        // next animation step
        animation_next_step();
        // animate sprites
        actors_animate(anim, 
                       (FIELD_OFFSET_X << 3) + DEVICE_SPRITE_PX_OFFSET_X + (selected_x << 4), 
                       (FIELD_OFFSET_Y << 3) + DEVICE_SPRITE_PX_OFFSET_Y + (selected_y << 4) - animation[anim]);
        // animate playfield
        playfield_process_animation(anim);

        // wait vblank
        wait_vbl_done();
    }
}
