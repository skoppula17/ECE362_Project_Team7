
#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>
#include <stdbool.h>
#include "game_state.h"

#define SNAKE_GRID_WIDTH 24
#define SNAKE_GRID_HEIGHT 32
#define SNAKE_MAX_LENGTH (SNAKE_GRID_WIDTH * SNAKE_GRID_HEIGHT)

typedef struct {
    int x;
    int y;
} snake_position_t;

// Initialize or reset the game and stop old sounds. Call audio_init() first.
void snake_init(void);

// Start movement with a full interval before the first step.
void snake_start(void);

// Update movement, food, collisions, and score.
void snake_update(void);

// Request a new movement direction.
void snake_set_direction(snake_direction_t direction);

// Access snake positions for LCD rendering.
int snake_get_length(void);
snake_position_t snake_get_segment(int index);

// Access current food position.
snake_position_t snake_get_food(void);

// Test helper: place food at a specific unoccupied cell.
bool snake_test_set_food(int x, int y);

#endif
