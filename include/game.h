#ifndef __GAME_H_INCLUDE__
#define __GAME_H_INCLUDE__

#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#include <string.h>
#include <stdlib.h>

#include "game_types.h"
#include "common_utils.h"
#include "score.h"
#include "playfield.h"
#include "sound.h"
#include "graphics.h"
#include "lee.h"

extern metasprite_t item[3];

game_state_e game_run(void);

#endif
