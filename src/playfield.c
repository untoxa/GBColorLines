#include <string.h>
#include <stdlib.h>

#include "playfield.h"
#include "score.h"
#include "sound.h"
#include "graphics.h"

UBYTE playfield[PLAYFIELD_SIZE];

UBYTE preview_colors[PREVIEW_SIZE];
UWORD preview_coords[PREVIEW_SIZE];
metasprite_t preview_items[PREVIEW_SIZE][3];

UBYTE playfield_anim = 0;

void playfield_draw() {
    for (UBYTE i = 0; i < (PLAYFIELD_HEIGHT * 2); i+=2) {
        set_bkg_tiles_blank(0, i, 18, 2, field_row);
    }
}

void playfield_draw_item(UBYTE x, UBYTE y, UBYTE color) {
    UBYTE attr = color & 0x07u;
    UBYTE attributes[4] = {attr, attr, attr, attr};
    UBYTE tiles[4] = {(attr << 2) + 0x01, (attr << 2) + 0x03, (attr << 2) + 0x02, (attr << 2) + 0x04};
    set_attributed_bkg_tiles(x << 1, y << 1, 2, 2, tiles, attributes);    
}

void playfield_draw_hint_item(UBYTE x, UBYTE y, UBYTE color) {
    UBYTE attr = color & 0x07u;
    UBYTE attributes[4] = {attr, attr, attr, attr};
    UBYTE tiles[4] = {(attr << 2) + 0x1d, (attr << 2) + 0x1f, (attr << 2) + 0x1e, (attr << 2) + 0x20};
    set_attributed_bkg_tiles(x << 1, y << 1, 2, 2, tiles, attributes);    
}

UBYTE playfield_put_item(UWORD idx, UBYTE color) {
    typedef struct {
        UBYTE x, y, count;
    } lineprop_t;

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

UWORD playfield_get_random_coord() {
    // get new coords
    UBYTE r = myrand(&r81) + 1;
    UBYTE exit = FALSE;
    UBYTE * pf = playfield;
    while (r) {
        if (*pf == 0) {
            if (--r == 0) break;
            exit = FALSE;
        }
        if (++pf == (playfield + sizeof(playfield))) {
            if (exit) return 100;
            pf = playfield;
            exit = TRUE;
        }
    }
    return (UWORD)(pf - playfield);
}

void playfield_refresh_preview() {
    for (UBYTE i = 0; i != PREVIEW_SIZE; i++) {
        // put item color
        preview_items[i][1].props = preview_items[i][0].props = preview_colors[i]; 
        // put coord hint
        UWORD idx = preview_coords[i];        
        if (idx < 81) playfield_draw_hint_item(idx % PLAYFIELD_WIDTH, idx / PLAYFIELD_HEIGHT, preview_colors[i]);
    }
}

void playfield_randomize_preview() {
    for (UBYTE i = 0; i != PREVIEW_SIZE; i++) {
        // put item color
        preview_colors[i] = myrand(&r7) + 1; 
        // put coord hint        
        preview_coords[i] = playfield_get_random_coord();
    }
    playfield_refresh_preview();
}

void playfield_process_animation(UBYTE anim) {
    if (anim & 1) {
        playfield_anim++; playfield_anim &= ANIM_MASK;
    }
    for (UBYTE i = 0; i != PREVIEW_SIZE; i++) {
        move_metasprite(preview_items[i], (preview_colors[i] - 1) << 2, (i << 1) + 4, 151 + animation[(playfield_anim + (i << 1)) & ANIM_MASK], 64 + (i << 4));
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

UBYTE playfield_put_random(UBYTE count, UBYTE color) {
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
        score_add(playfield_put_item((UWORD)(pf - playfield), (color) ? color : myrand(&r7) + 1));       
    }

    // pop sound
    SOUND_POP;

    // check for free space
    for (UBYTE i = 0; i < PLAYFIELD_SIZE; i++)
        if (playfield[i] == 0) return TRUE;

    return FALSE;
}

UBYTE playfield_put_previewed() {
    UBYTE not_put[PREVIEW_SIZE];
    UBYTE put_count = 0;
    for (UBYTE i = 0; i != PREVIEW_SIZE; i++) {
        UWORD idx = preview_coords[i];
        if ((idx < 81) && (playfield[idx] == 0)) {
            put_count++;
            score_add(playfield_put_item(idx, preview_colors[i]));
            not_put[i] = 0;
        } else {
            not_put[i] = preview_colors[i];
        }
    }

    for (UBYTE i = 0; i != PREVIEW_SIZE; i++)
        if (not_put[i]) 
            if (!playfield_put_random(1, not_put[i])) return FALSE;

    // pop sound
    SOUND_POP;

    // check for free space
    for (UBYTE i = 0; i < PLAYFIELD_SIZE; i++)
        if (playfield[i] == 0) return TRUE;

    return FALSE;
}
