#include "lee.h"

#include <string.h>

#define MAX_QUEUE_LENGTH LEE_MAX_WIDTH * LEE_MAX_HEIGHT

UWORD queue[MAX_QUEUE_LENGTH]; 
UWORD queue_head, queue_tail;

inline void queue_reset(void) {
    queue_head = queue_tail = 0;
}

inline UBYTE queue_empty(void) { 
    return (queue_head == queue_tail); 
}

inline UBYTE queue_push(UWORD data) {
    if (++queue_head == MAX_QUEUE_LENGTH) queue_head = 0;
    if (queue_head != queue_tail) {
        queue[queue_head] = data;
        return TRUE;
    }
    return FALSE;
}

inline UWORD queue_pop(void) {
    if (++queue_tail == MAX_QUEUE_LENGTH) queue_tail = 0;
    return queue[queue_tail];
}

UBYTE visited[LEE_MAX_WIDTH * LEE_MAX_HEIGHT];

extern UBYTE lee_test_collision(UBYTE x, UBYTE y);

UBYTE lee_find_path(UBYTE x, UBYTE y, UBYTE dx, UBYTE dy) {
    static UWORD src, dest, pos;
    static UBYTE step;

    src = (((y & (LEE_MAX_WIDTH - 1)) * LEE_MAX_WIDTH) + (x & (LEE_MAX_WIDTH - 1)));
    dest = (((dy & (LEE_MAX_WIDTH - 1)) * LEE_MAX_WIDTH) + (dx & (LEE_MAX_WIDTH - 1)));

    queue_reset();
    memset(visited, LEE_MAX_STEPS, sizeof(visited));

    step = 0;
    visited[src] = step;  
    queue_push(src); 

    while (!queue_empty()) {
        pos = queue_pop();
        step = visited[pos];
        if (pos == dest) return step;

        step++;
        if (step == LEE_MAX_STEPS) return LEE_MAX_STEPS;

        UBYTE cx = lee_get_coords_x(pos);
        UBYTE cy = lee_get_coords_y(pos);

        if ((cx != 0) && (visited[pos - 1] == LEE_MAX_STEPS) && (lee_test_collision(cx - 1, cy))) {
            visited[pos - 1] = step;
            queue_push(pos - 1);
        }
        if ((cx < (LEE_MAX_WIDTH - 1)) && (visited[pos + 1] == LEE_MAX_STEPS) && (lee_test_collision(cx + 1, cy))) {
            visited[pos + 1] = step;
            queue_push(pos + 1);
        }
        if ((cy != 0) && (visited[pos - LEE_MAX_WIDTH] == LEE_MAX_STEPS) && (lee_test_collision(cx, cy - 1))) {
            visited[pos - LEE_MAX_WIDTH] = step;
            queue_push(pos - LEE_MAX_WIDTH);
        }
        if ((cy < (LEE_MAX_HEIGHT - 1)) && (visited[pos + LEE_MAX_WIDTH] == LEE_MAX_STEPS) && (lee_test_collision(cx, cy + 1))) {
            visited[pos + LEE_MAX_WIDTH] = step;
            queue_push(pos + LEE_MAX_WIDTH);
        }
    }
    return LEE_MAX_STEPS; 
}

UBYTE lee_restore_path(UBYTE x, UBYTE y, UWORD * path) {
    static UWORD pos;
    static UBYTE step, cx, cy;

    cx = x; cy = y;
    pos = (((y & (LEE_MAX_WIDTH - 1)) * LEE_MAX_WIDTH) + (x & (LEE_MAX_WIDTH - 1)));

    step = visited[pos];
    while (step) {
        if (step == LEE_MAX_STEPS) return LEE_MAX_STEPS;
        path[step] = pos;

        if ((cx != 0) && (visited[pos - 1] < step)) {
            pos--; cx--;
            step = visited[pos];
            continue;
        } 
        if ((cx < (LEE_MAX_WIDTH - 1)) && (visited[pos + 1] < step)) {
            pos++; cx++;
            step = visited[pos];
            continue;
        } 
        if ((cy != 0) && (visited[pos - LEE_MAX_WIDTH] < step)) {
            pos -= LEE_MAX_WIDTH; cy--;
            step = visited[pos];
            continue;
        } 
        if ((cy < (LEE_MAX_HEIGHT - 1)) && (visited[pos + LEE_MAX_WIDTH] < step)) {
            pos += LEE_MAX_WIDTH; cy++;
            step = visited[pos];
            continue;
        }
        return LEE_MAX_STEPS;
    }
    path[step] = pos;
    return 0;
}
