#ifndef _SOUND_SAMPLEPLAYER_H_INCLUDE
#define _SOUND_SAMPLEPLAYER_H_INCLUDE

#include <gbdk/platform.h>

extern UINT8 play_bank;
extern const UINT8 * play_sample;
extern UINT16 play_length;

void set_sample(UINT8 bank, const UINT8 * sample, UINT16 length) NONBANKED;
void play_isr(void) NONBANKED;

#endif