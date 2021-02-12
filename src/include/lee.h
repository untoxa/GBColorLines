#ifndef _LEE_H_INCLUDE
#define _LEE_H_INCLUDE

#include <gb/gb.h>

#define LEE_MAX_STEPS 255u

#define LEE_BITS 5
#define LEE_MASK 0x1fu

#define LEE_MAX_WIDTH (1 << LEE_BITS)
#define LEE_MAX_HEIGHT (1 << LEE_BITS) 

UBYTE lee_find_path(UBYTE x, UBYTE y, UBYTE dx, UBYTE dy);
UBYTE lee_restore_path(UBYTE x, UBYTE y, UWORD * path);

inline UBYTE lee_get_coords_x(UWORD pos) {
    return pos & LEE_MASK;
}
inline UBYTE lee_get_coords_y(UWORD pos) {
    return pos >> LEE_BITS;
}

#endif