#include "myrand.h"

#include <rand.h>

void randomize() {
    initarand(DIV_REG);
}

UINT8 myrand(myrand_state_t * state) {
    UINT16 v = state->previous + arand() & state->modulus;
    UINT8 m = state->maxvalue;
    if (v >= m) v -= m;
    if (v >= m) v -= m;
    return state->previous = (UINT8)v;
}

void myrand_init(UINT8 avalue, myrand_state_t * state) {
    UINT8 m = 0, i = avalue;
    do {
        m |= i;
        i = i >> 1;
    } while (i != 0);
    state->previous = 0; state->modulus = m; state->maxvalue = avalue;
    myrand(state);
}
