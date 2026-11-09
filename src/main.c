#include "audio.h"
#include "game_state.h"
#include "pico/stdlib.h"

#ifndef AUDIO_TEST_MODE
#define AUDIO_TEST_MODE 0
#endif

game_state_t game_state = GAME_WAITING;
snake_direction_t game_requested_direction = DIRECTION_RIGHT;
uint32_t game_score = 0;
uint16_t game_joystick_x_raw = 0;
uint16_t game_joystick_y_raw = 0;
bool game_food_audio_event = false;
bool game_over_audio_event = false;

#if AUDIO_TEST_MODE
#define AUDIO_TEST_PAUSE_MS 1000u

typedef enum {
    AUDIO_TEST_START,
    AUDIO_TEST_WAIT_FOOD,
    AUDIO_TEST_PAUSE,
    AUDIO_TEST_WAIT_GAME_OVER,
    AUDIO_TEST_DONE
} audio_test_state_t;

static void audio_test_update(void)
{
    static audio_test_state_t state = AUDIO_TEST_START;
    static absolute_time_t pause_deadline;

    switch (state) {
    case AUDIO_TEST_START:
        audio_play_food();
        state = AUDIO_TEST_WAIT_FOOD;
        break;
    case AUDIO_TEST_WAIT_FOOD:
        if (!audio_is_busy()) {
            pause_deadline = make_timeout_time_ms(AUDIO_TEST_PAUSE_MS);
            state = AUDIO_TEST_PAUSE;
        }
        break;
    case AUDIO_TEST_PAUSE:
        if (time_reached(pause_deadline)) {
            audio_play_game_over();
            state = AUDIO_TEST_WAIT_GAME_OVER;
        }
        break;
    case AUDIO_TEST_WAIT_GAME_OVER:
        if (!audio_is_busy()) {
            state = AUDIO_TEST_DONE;
        }
        break;
    case AUDIO_TEST_DONE:
        break;
    }
}
#endif

int main(void)
{
    audio_init();
    // TODO: Joystick initialization
    // TODO: TFT initialization (
    // TODO: PIO score display initialization

    while (true) {
        // TODO: Joystick update
        // TODO: Game update: state, score, and one audio event per food/collision.
        // Wall and self-collisions both set game_over_audio_event once.

#if AUDIO_TEST_MODE
        audio_test_update();
#endif

        if (game_over_audio_event) {
            game_over_audio_event = false;
            game_food_audio_event = false;
            audio_play_game_over();
        } else if (game_food_audio_event) {
            game_food_audio_event = false;
            audio_play_food();
        }

        audio_update();

        // TODO: TFT rendering
        // TODO: PIO score display update
        tight_loop_contents();
    }
}
