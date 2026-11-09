#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    GAME_WAITING,
    GAME_RUNNING,
    GAME_OVER
} game_state_t;

typedef enum {
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
} snake_direction_t;

// Defined once in main.c. Access only from the single-core main loop.
// Interrupts, DMA, or another core need coordinated access, not just volatile.
extern game_state_t game_state;
extern snake_direction_t game_requested_direction;
extern uint32_t game_score;

// Raw ADC readings, not normalized. Zero initially means not sampled yet.
extern uint16_t game_joystick_x_raw;
extern uint16_t game_joystick_y_raw;

// Set once per event. The main loop consumes and clears these flags.
extern bool game_food_audio_event;
extern bool game_over_audio_event;

#endif
