#ifndef _SOUND_H_INCLUDE
#define _SOUND_H_INCLUDE

#include <gb/gb.h>

#define ENABLE_CURSOR_SOUND 1

extern UBYTE music_playing;
extern UBYTE sound_playing;

extern UBYTE channel_pause[4];

void music_play();
void music_stop();

void music_update();

#if ENABLE_CURSOR_SOUND != 0
    #define SOUND_CURSOR sound_play(0,  5,  0x20, 0x01, 0xA3, 0x73, 0x86)
#else
    #define SOUND_CURSOR
#endif

#define sound_priority(ch) channel_pause[(ch)]=0
#define sound_play_priority(ch, ...) sound_priority((ch)), sound_play((ch), __VA_ARGS__)

#define SOUND_BALL   sound_play(0, 10,  0x23, 0x80, 0xA2, 0x67, 0x83)
#define SOUND_CLEAR  sound_play_priority(0, 45,  0x5F, 0x80, 0xA6, 0x24, 0x86)
#define SOUND_POP    sound_play_priority(0, 49,  0x37, 0x80, 0xA6, 0x23, 0x86)

void sound_play(UBYTE channel, UBYTE mute_frames, ...);

#endif