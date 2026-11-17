#include "joystick.h"
#include "hardware/adc.h"
#include "pico/time.h"
#include <stdlib.h>
#if JOYSTICK_DEBUG
#include <stdio.h>
#endif

_Static_assert(JOYSTICK_X_PIN >= 40 && JOYSTICK_X_PIN <= 47,
               "Proton joystick X must use GPIO40-47");
_Static_assert(JOYSTICK_Y_PIN >= 40 && JOYSTICK_Y_PIN <= 47,
               "Proton joystick Y must use GPIO40-47");
_Static_assert(JOYSTICK_X_PIN != JOYSTICK_Y_PIN, "Use separate joystick pins");

enum { INPUT_CENTER = 4 };
static bool ready;
static int center_x, center_y;
static unsigned int calibration_count;
static uint32_t sum_x, sum_y;
static uint16_t min_x, max_x, min_y, max_y;
static int accepted, candidate;
static absolute_time_t next_sample, candidate_deadline;

void joystick_init(void)
{
    ready = false;
    calibration_count = 0;
    accepted = candidate = INPUT_CENTER;
    next_sample = nil_time;
    game_joystick_centered = false;
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
}

bool joystick_is_ready(void) { return ready; }

static uint16_t read_axis(unsigned int channel)
{
    adc_select_input(channel);
    (void)adc_read(); // Discard the first conversion after switching axes.
    uint32_t sum = 0;
    for (int i = 0; i < 4; i++) sum += adc_read();
    return (uint16_t)(sum / 4);
}

static void calibrate(uint16_t x, uint16_t y)
{
    // Do not learn an end-stop or disconnected rail as the neutral position.
    if (x < 512 || x > 3583 || y < 512 || y > 3583) {
        calibration_count = 0;
        return;
    }
    if (calibration_count == 0) {
        sum_x = sum_y = 0;
        min_x = max_x = x;
        min_y = max_y = y;
    }
    if (x < min_x) min_x = x;
    if (x > max_x) max_x = x;
    if (y < min_y) min_y = y;
    if (y > max_y) max_y = y;
    if (max_x - min_x > 100 || max_y - min_y > 100) {
        calibration_count = 0;
        return;
    }
    sum_x += x;
    sum_y += y;
    if (++calibration_count == JOYSTICK_CALIBRATION_SAMPLES) {
        center_x = (int)(sum_x / calibration_count);
        center_y = (int)(sum_y / calibration_count);
        ready = true;
#if JOYSTICK_DEBUG
        printf("Joystick center X=%d Y=%d\n", center_x, center_y);
#endif
    }
}

static int classify(int x, int y)
{
    int ax = abs(x), ay = abs(y);
    if (ax < JOYSTICK_RELEASE_DEADZONE && ay < JOYSTICK_RELEASE_DEADZONE)
        return INPUT_CENTER;
    // Keep the current axis while returning to center; reject diagonal jitter.
    if (((accepted == DIRECTION_RIGHT && x > 0) ||
         (accepted == DIRECTION_LEFT && x < 0)) &&
        ax >= JOYSTICK_RELEASE_DEADZONE && ay < ax + JOYSTICK_AXIS_MARGIN)
        return accepted;
    if (((accepted == DIRECTION_DOWN && y > 0) ||
         (accepted == DIRECTION_UP && y < 0)) &&
        ay >= JOYSTICK_RELEASE_DEADZONE && ax < ay + JOYSTICK_AXIS_MARGIN)
        return accepted;
    if (ax < JOYSTICK_DEADZONE && ay < JOYSTICK_DEADZONE)
        return INPUT_CENTER;
    if (ax >= ay) return x > 0 ? DIRECTION_RIGHT : DIRECTION_LEFT;
    return y > 0 ? DIRECTION_DOWN : DIRECTION_UP;
}

bool joystick_read(snake_direction_t *direction)
{
    if (time_reached(next_sample)) {
        next_sample = make_timeout_time_ms(JOYSTICK_SAMPLE_MS);
        game_joystick_x_raw = read_axis(JOYSTICK_X_PIN - 40);
        game_joystick_y_raw = read_axis(JOYSTICK_Y_PIN - 40);
        if (!ready) {
            calibrate(game_joystick_x_raw, game_joystick_y_raw);
        } else {
            int x = (int)game_joystick_x_raw - center_x;
            int y = (int)game_joystick_y_raw - center_y;
            if (JOYSTICK_INVERT_X) x = -x;
            if (JOYSTICK_INVERT_Y) y = -y;
            int next = classify(x, y);
            if (next != candidate) {
                candidate = next;
                candidate_deadline = make_timeout_time_ms(JOYSTICK_DIRECTION_STABLE_MS);
            } else if (candidate != accepted && time_reached(candidate_deadline)) {
                accepted = candidate;
            }
        }
    }
    if (!ready) return false;
    if (accepted != INPUT_CENTER) *direction = (snake_direction_t)accepted;
    return accepted == INPUT_CENTER;
}
