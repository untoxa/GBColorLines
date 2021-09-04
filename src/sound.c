#include <gbdk/platform.h>
#include <gb/isr.h>

#include <string.h>

#include "hUGEDriver.h"
#include "sound_sampleplayer.h"
#include "sound.h"

extern const hUGESong_t BGM_MAIN;

UBYTE music_initialized = FALSE;
UBYTE music_playing = FALSE;
UBYTE sound_playing = TRUE;
UBYTE channel_pause[4];

UBYTE sound_control = SOUND_ON | MUSIC_ON;

void toggle_sound_settings(UBYTE addend) {
    sound_control += addend; sound_control &= (SOUND_ON | MUSIC_ON); 
    if (sound_control & MUSIC_ON) music_play(); else music_stop();
    sound_playing = (sound_control & SOUND_ON);
}

void music_init() {
    if (music_initialized) return;
    memset(channel_pause, 0, sizeof(channel_pause));
    hUGE_init(&BGM_MAIN);
    music_initialized = TRUE;
}

UINT8 ISR_counter = 0;
static void music_update_data() {
    play_isr();
    ISR_counter++; ISR_counter &= 3;
    if (ISR_counter) return;
    
    // resume channel
    for (UBYTE i = HT_CH1; i <= HT_CH4; i++) {
        if (channel_pause[i]) {
            if (--channel_pause[i]) continue; 
            if (music_playing) hUGE_mute_channel(i, HT_CH_PLAY);
        }
    }
    // play sound
    if (music_playing) hUGE_dosound();
}
void music_update() __naked {
__asm
        push af
        push hl
        push bc
        push de

        call _music_update_data

        pop de
        pop bc
        pop hl
1$:
        ldh a, (_STAT_REG)
        and #STATF_BUSY
        jr nz, 1$        
        pop af
        reti
__endasm;
}
ISR_NESTED_VECTOR(VECTOR_TIMER, music_update)

void music_play() {
    music_init();
    for (UBYTE i = HT_CH1; i <= HT_CH4; i++)
        hUGE_mute_channel(i, HT_CH_PLAY);
    music_playing = TRUE;
}

void music_stop() {
    music_playing = FALSE;
    music_init();
    for (UBYTE i = HT_CH1; i <= HT_CH4; i++)
        hUGE_mute_channel(i, HT_CH_MUTE);
}

const UINT8 const FX_REG_SIZES[] = {5, 4, 5, 4, 3};
const UINT8 const FX_ADDR_LO[]   = {0x10, 0x16, 0x1A, 0x20, 0x24};

void sound_play(UBYTE channel, UINT8 mute_frames, ...) __naked {
    channel; mute_frames;
__asm
            ld      A, (#_sound_playing)
            or      A
            ret     Z

            push    BC
            
            ldhl    SP, #4
            ld      B, #0 
            ld      A, (HL+)           
            ld      C, A        ; BC = channel

            ld      E, (HL)
            ld      HL, #_channel_pause
            add     HL, BC
            ld      A, (HL)
            or      A
            jr      NZ, 2$
            ld      (HL), E

            inc     B
            push    BC
            call    _hUGE_mute_channel
            pop     BC
            dec     B

            ld      HL, #_FX_REG_SIZES
            add     HL, BC
            ld      E, (HL)     ; E = FX_REG_SIZES[channel]
            
            ld      HL, #_FX_ADDR_LO
            add     HL, BC
            ld      B, #0xFF
            ld      C, (HL)     ; BC = 0xFF00 + FX_ADDR_LO[channel]
            
            lda     HL, 6(SP) // varargs
1$:
            ld      A, (HL+)
            inc     HL
            ld      (BC), A
            inc     BC
            dec     E
            
            jr      NZ, 1$
2$:
            pop     BC
            ret
__endasm;
}