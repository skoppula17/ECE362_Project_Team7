"""Run with python3 tests/test_live_input.py; uses cc and mocked hardware."""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "game.h"
#include "joystick.h"
#include "lcd.h"
#include "snake.h"
uint64_t now;
game_state_t game_state;
snake_direction_t game_requested_direction;
uint32_t game_score;
uint16_t game_joystick_x_raw, game_joystick_y_raw;
bool game_joystick_centered, game_food_audio_event, game_over_audio_event;
static uint16_t raw_x, raw_y;
static unsigned int channel;
void adc_init(void) {}
void adc_gpio_init(unsigned int p) { assert(p == 41 || p == 42); }
void adc_select_input(unsigned int c) { assert(c == 1 || c == 2); channel = c; }
uint16_t adc_read(void) { return channel == 1 ? raw_x : raw_y; }
uint32_t get_rand_32(void) { return 100; }
void audio_stop(void) {}
void lcd_fill_screen(uint16_t c) {}
void lcd_draw_text(int x, int y, const char *t, uint16_t c, int scale) {}
void lcd_draw_cell(int x, int y, uint16_t c) {
    assert(x >= 0 && x < 24 && y >= 0 && y < 32);
}
#include "src/joystick.c"
#include "src/snake.c"
#include "src/game.c"
#include "src/lcd_tests.c"
static snake_direction_t input_direction;
static void sample(int x, int y) {
    // Deliberately offset neutral from 2048 to verify actual calibration.
    raw_x = (uint16_t)(2600 + (JOYSTICK_INVERT_X ? -x : x));
    raw_y = (uint16_t)(1600 + (JOYSTICK_INVERT_Y ? -y : y));
    now += JOYSTICK_SAMPLE_MS;
    input_direction = game_requested_direction;
    game_joystick_centered = joystick_read(&input_direction);
}
static void hold(int x, int y) { for (int i = 0; i < 6; i++) sample(x, y); }
static void tick(int x, int y) {
    hold(x, y);
    game_update(game_joystick_centered, input_direction);
}
int main(int argc, char **argv) {
    if (argc > 1) {
        int wall = atoi(argv[1]);
        for (int i = 0; i < 10000 && game_state != GAME_OVER; i++) {
            now += 10;
            if (wall) lcd_complete_wall_game_test_update();
            else lcd_complete_game_test_update();
        }
        assert(game_state == GAME_OVER && game_over_audio_event);
        assert(game_score == (wall ? 5 : 3));
        if (wall) lcd_complete_wall_game_test_update();
        else lcd_complete_game_test_update();
        puts(wall ? "PASS: new five-food wall demo" : "PASS: three-food self-collision demo");
        return 0;
    }
    joystick_init();
    for (int i = 0; i < 40; i++) {
        raw_x = raw_y = 4095; now += 5; joystick_read(&input_direction);
    }
    assert(!joystick_is_ready());
    for (int i = 0; i < 32; i++) sample(0, 0);
    assert(joystick_is_ready() && game_joystick_centered);
    hold(1000, 0); assert(input_direction == DIRECTION_RIGHT);
    sample(-1000, 0); sample(-1000, 0);
    assert(input_direction == DIRECTION_RIGHT); // Release spike rejected.
    hold(0, 0); assert(game_joystick_centered);
    hold(-1000, 0); assert(input_direction == DIRECTION_LEFT);
    hold(0, -1000); assert(input_direction == DIRECTION_UP);
    hold(0, 1000); assert(input_direction == DIRECTION_DOWN);
    game_init();
    tick(1000, 0); assert(game_state == GAME_WAITING);
    tick(0, 0); tick(1000, 0); assert(game_state == GAME_RUNNING);
    assert(snake_test_set_food(10, 10));
    now += 250; tick(0, 0); assert(game_score == 1 && game_food_audio_event);
    now += 250; tick(-1000, 0); assert(snake_get_segment(5).x == 11);
    while (game_state == GAME_RUNNING) { now += 250; tick(1000, 0); }
    tick(1000, 0); assert(game_state == GAME_OVER);
    tick(0, 0); tick(0, -1000);
    assert(game_state == GAME_RUNNING && game_score == 0 && !game_over_audio_event);
    now += 250; tick(0, 0); assert(snake_get_segment(4).y == 9);
    snake_init(); snake_start();
    snake_set_direction(DIRECTION_UP); snake_set_direction(DIRECTION_LEFT);
    now += 250; snake_update();
    now += 250; snake_update();
    assert(snake_get_segment(4).x == 9 && snake_get_segment(4).y == 8);
    puts("PASS: calibration, directions, release spikes, queued turns, start, food, collision, restart");
}
'''
with tempfile.TemporaryDirectory(prefix="snake-test-") as directory:
    temp = Path(directory)
    (temp / 'hardware').mkdir()
    (temp / 'pico').mkdir()
    (temp / 'hardware/adc.h').write_text(
        '#include <stdint.h>\nvoid adc_init(void);\nvoid adc_gpio_init(unsigned int);\n'
        'void adc_select_input(unsigned int);\nuint16_t adc_read(void);\n')
    (temp / 'pico/stdlib.h').write_text(
        '#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n'
        'typedef uint64_t absolute_time_t;\nextern uint64_t now;\n#define nil_time 0\n'
        'static inline bool time_reached(absolute_time_t t) { return now >= t; }\n'
        'static inline absolute_time_t make_timeout_time_ms(uint32_t t) { return now+t; }\n')
    (temp / 'pico/time.h').write_text('#include "pico/stdlib.h"\n')
    (temp / 'pico/rand.h').write_text('#include <stdint.h>\nuint32_t get_rand_32(void);\n')
    (temp / 'test.c').write_text(SOURCE)
    for invert in (0, 1):
        subprocess.run(['cc', '-std=c11', '-DJOYSTICK_DEBUG=0',
                        f'-DJOYSTICK_INVERT_X={invert}', f'-DJOYSTICK_INVERT_Y={invert}',
                        f'-I{temp}', f'-I{ROOT}', f'-I{ROOT / "include"}',
                        str(temp / 'test.c'), '-o', str(temp / 'test')], check=True)
        for args in ([], ['0'], ['1']):
            subprocess.run([str(temp / 'test'), *args], check=True)
