#ifndef GAME_H
#define GAME_H
#include "game_state.h"

// Call after initializing the LCD and audio.
void game_init(void);
// Supply a fresh joystick sample; center then move to start/restart.
void game_update(bool centered, snake_direction_t direction);
#endif
