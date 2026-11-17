#ifndef JOYSTICK_H
#define JOYSTICK_H
#include "game_state.h"

// Proton RP2350B ADC channels 1/2: connect X to GP41, Y to GP42.
#ifndef JOYSTICK_X_PIN
#define JOYSTICK_X_PIN 41u
#endif
#ifndef JOYSTICK_Y_PIN
#define JOYSTICK_Y_PIN 42u
#endif
#ifndef JOYSTICK_INVERT_X
#define JOYSTICK_INVERT_X 0
#endif
#ifndef JOYSTICK_INVERT_Y
#define JOYSTICK_INVERT_Y 0
#endif
// Leave the stick released at startup to learn its resting center.
#define JOYSTICK_SAMPLE_MS 5u
#define JOYSTICK_CALIBRATION_SAMPLES 32u
#define JOYSTICK_DEADZONE 600
#define JOYSTICK_RELEASE_DEADZONE 400
#define JOYSTICK_DIRECTION_STABLE_MS 20u
#define JOYSTICK_AXIS_MARGIN 250
#ifndef JOYSTICK_DEBUG
#define JOYSTICK_DEBUG 1
#endif

void joystick_init(void);
bool joystick_is_ready(void);
// Nonblocking polling. False until calibrated. True when centered; otherwise
// writes the stable direction. Raw ADC values are available in game_state.h.
bool joystick_read(snake_direction_t *direction);
#endif
