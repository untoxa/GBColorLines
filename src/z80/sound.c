#include <gbdk/platform.h>

#include "sound.h"

UBYTE music_initialized = FALSE;
UBYTE music_playing = FALSE;
UBYTE sound_playing = TRUE;

UBYTE sound_control = SOUND_ON | MUSIC_ON;

void sound_init() {
}

void toggle_sound_settings(UBYTE addend) {
    sound_control += addend; sound_control &= (SOUND_ON | MUSIC_ON); 
    if (sound_control & MUSIC_ON) music_play(); else music_stop();
    sound_playing = (sound_control & SOUND_ON);
}

void music_init() {
}

void music_update() {
}

void music_play() {
}

void music_stop() {
}

void sound_play(UBYTE channel, UINT8 mute_frames, ...) {
    channel; mute_frames;
}