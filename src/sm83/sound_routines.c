#include "gbdk/platform.h"

#include "sound_sampleplayer.h"
#include "sound_samples.h"
#include "sound_routines.h"

void Routine0(unsigned char param, unsigned char ch, unsigned char tick) OLDCALL {
    ch; tick; // suppress warinigs
    const wave_sample_t * sample;
    if (tick) return;
    sample = wave_samples + (param >> 4);
    play_bank = sample->bank;
    play_sample = sample->data;
    play_length = sample->length;
}

const hUGERoutine_t routines[] = { Routine0 };
