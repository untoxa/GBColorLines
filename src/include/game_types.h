#ifndef _GAME_TYPES_H_INCLUDE
#define _GAME_TYPES_H_INCLUDE

#define PLAYFIELD_WIDTH 9u
#define PLAYFIELD_HEIGHT 9u
#define PLAYFIELD_SIZE (PLAYFIELD_HEIGHT * PLAYFIELD_WIDTH) 

#define PREVIEW_SIZE 3
#define PREVIEW_FLAG 0x80u
#define PREVIEW_MASK ~PREVIEW_FLAG

#define SCORE_SIZE 5

#define ANIM_MASK 0x0fu

typedef enum {game_intro, game_play, game_over} game_state_e;

#define HIGHSCORE_SIGNATURE 0x45564153ul
typedef struct sram_record_t {
    UINT32 signature;
    UWORD highscore;
} sram_record_t;

#endif