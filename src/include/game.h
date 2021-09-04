#ifndef __GAME_H_INCLUDE__
#define __GAME_H_INCLUDE__

#include <gbdk/platform.h>
#include <gb/metasprites.h>

#include <string.h>
#include <stdlib.h>

#include "game_types.h"
#include "common_utils.h"
#include "score.h"
#include "playfield.h"
#include "sound.h"
#include "graphics.h"
#include "myrand.h"
#include "lee.h"

extern myrand_state_t r7, r81;
extern metasprite_t item[3];

game_state_e game_run();

#endif
