#ifndef _SOUND_SAMPLES_H_INCLUDE
#define _SOUND_SAMPLES_H_INCLUDE

#include <gbdk/platform.h>

typedef struct {
    UBYTE bank;
    const UINT8 * data;
    UINT16 length;
} wave_sample_t;

extern const wave_sample_t wave_samples[]; 

#endif