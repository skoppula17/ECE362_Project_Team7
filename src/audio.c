#include "audio.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/time.h"

#define AUDIO_PIN 36u
#define AUDIO_FREQUENCY_HZ 2400u
#define AUDIO_FOOD_MS 100u
#define AUDIO_GAME_OVER_BEEP_MS 100u
#define AUDIO_GAME_OVER_GAP_MS 100u
#define AUDIO_GAME_OVER_BEEPS 3u
#define PWM_MAX_PERIOD_TICKS 65536u
#define PWM_MAX_INTEGER_DIVIDER 255u

typedef enum {
    AUDIO_IDLE,
    AUDIO_FOOD,
    AUDIO_GAME_OVER
} audio_state_t;

static audio_state_t audio_state = AUDIO_IDLE;
static uint audio_slice;
static uint audio_channel;
static uint16_t audio_level;
static uint8_t beeps_remaining;
static bool tone_active;
static absolute_time_t phase_deadline;

static void silence_output(void)
{
    // Stopping a PWM counter alone can leave its output HIGH. Use SIO LOW first.
    gpio_put(AUDIO_PIN, false);
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_SIO);
    pwm_set_enabled(audio_slice, false);
    pwm_set_chan_level(audio_slice, audio_channel, 0);
    tone_active = false;
}

static void start_beep(uint32_t duration_ms)
{
    silence_output();
    pwm_set_counter(audio_slice, 0);
    pwm_set_chan_level(audio_slice, audio_channel, audio_level);
    pwm_set_enabled(audio_slice, true);
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
    tone_active = true;
    phase_deadline = make_timeout_time_ms(duration_ms);
}

void audio_init(void)
{
    gpio_init(AUDIO_PIN);
    gpio_put(AUDIO_PIN, false);
    gpio_set_dir(AUDIO_PIN, GPIO_OUT);


    audio_slice = pwm_gpio_to_slice_num(AUDIO_PIN);
    audio_channel = pwm_gpio_to_channel(AUDIO_PIN);

    const uint32_t clock_hz = clock_get_hz(clk_sys);
    const uint64_t max_period_hz =
        (uint64_t)AUDIO_FREQUENCY_HZ * PWM_MAX_PERIOD_TICKS;
    uint32_t divider = (uint32_t)((clock_hz + max_period_hz - 1u) /
                                max_period_hz);
    if (divider < 1u) {
        divider = 1u;
    } else if (divider > PWM_MAX_INTEGER_DIVIDER) {
        divider = PWM_MAX_INTEGER_DIVIDER;
    }

    const uint32_t divided_frequency = divider * AUDIO_FREQUENCY_HZ;
    uint32_t period_ticks = (uint32_t)(((uint64_t)clock_hz +
                                      divided_frequency / 2u) /
                                     divided_frequency);
    if (period_ticks < 2u) {
        period_ticks = 2u;
    } else if (period_ticks > PWM_MAX_PERIOD_TICKS) {
        period_ticks = PWM_MAX_PERIOD_TICKS;
    }

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, divider);
    pwm_config_set_wrap(&config, (uint16_t)(period_ticks - 1u));
    pwm_init(audio_slice, &config, false);
    audio_level = (uint16_t)(period_ticks / 2u);
    audio_stop();
}

void audio_play_food(void)
{
    if (audio_state == AUDIO_GAME_OVER) {
        return;
    }

    audio_state = AUDIO_FOOD;
    beeps_remaining = 1u;
    start_beep(AUDIO_FOOD_MS);
}

void audio_play_game_over(void)
{
    if (audio_state == AUDIO_GAME_OVER) {
        return;
    }

    audio_state = AUDIO_GAME_OVER;
    beeps_remaining = AUDIO_GAME_OVER_BEEPS;
    start_beep(AUDIO_GAME_OVER_BEEP_MS);
}

void audio_update(void)
{
    if (audio_state == AUDIO_IDLE || !time_reached(phase_deadline)) {
        return;
    }

    if (tone_active) {
        silence_output();
        --beeps_remaining;
        if (beeps_remaining == 0u) {
            audio_stop();
        } else {

            phase_deadline = make_timeout_time_ms(AUDIO_GAME_OVER_GAP_MS);
        }
    } else {
        start_beep(AUDIO_GAME_OVER_BEEP_MS);
    }
}

void audio_stop(void)
{
    silence_output();
    audio_state = AUDIO_IDLE;
    beeps_remaining = 0u;
    phase_deadline = nil_time;
}

bool audio_is_busy(void)
{
    return audio_state != AUDIO_IDLE;
}
