#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

// Single-core, Initialize once before calling other functions.
void audio_init(void);

// One 100 ms buzz. Repeated food requests restart it
void audio_play_food(void);

// Three buzzes for either collision.
void audio_play_game_over(void);

// Call every loop
void audio_update(void);
bool audio_is_busy(void);

// Immediately drive GP36 LOW and cancel the entire sequence.
void audio_stop(void);

#endif
