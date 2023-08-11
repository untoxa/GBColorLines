#ifndef _MYRAND_H_INCLUDE
#define _MYRAND_H_INCLUDE

#include <gbdk/platform.h>

#define MAKEMASK(A) (A) | ((A)>>1) | ((A) >> 2) | ((A) >> 3)  | ((A) >> 4) | ((A) >> 5) | ((A) >> 6) | ((A) >> 7)

typedef struct {
    UINT8 previous, modulus, maxvalue;
} myrand_state_t;

void randomize(void);
UINT8 myrand(myrand_state_t * state);
void myrand_init(UINT8 avalue, myrand_state_t * state);

#endif