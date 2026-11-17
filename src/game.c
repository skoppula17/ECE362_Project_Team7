#include "game.h"
#include "snake.h"
#include "lcd.h"
#include <stdio.h>

static bool start_armed;
static game_state_t displayed_state;

static void draw_board(void)
{
    lcd_fill_screen(LCD_BLACK);
    int length = snake_get_length();
    for (int i = 0; i < length; i++) {
        snake_position_t p = snake_get_segment(i);
        lcd_draw_cell(p.x, p.y, i == length - 1 ? LCD_YELLOW : LCD_GREEN);
    }
    snake_position_t food = snake_get_food();
    lcd_draw_cell(food.x, food.y, LCD_RED);
}

void game_init(void)
{
    snake_init();
    start_armed = false;
    displayed_state = GAME_WAITING;
    lcd_fill_screen(LCD_BLACK);
    lcd_draw_text(30, 65, "SNAKE BYTE", LCD_GREEN, 3);
    lcd_draw_text(12, 155, "CENTER JOYSTICK", LCD_WHITE, 2);
    lcd_draw_text(12, 180, "THEN MOVE TO START", LCD_YELLOW, 2);
}

void game_update(bool centered, snake_direction_t direction)
{
    if (game_state == GAME_OVER && displayed_state != GAME_OVER) {
        start_armed = false;
        displayed_state = GAME_OVER;
        lcd_fill_screen(LCD_BLACK);
        lcd_draw_text(39, 80, "GAME OVER", LCD_RED, 3);
        char score[24];
        snprintf(score, sizeof(score), "SCORE %lu", (unsigned long)game_score);
        lcd_draw_text(48, 150, score, LCD_WHITE, 2);
        lcd_draw_text(12, 210, "CENTER JOYSTICK", LCD_GREEN, 2);
        lcd_draw_text(12, 235, "MOVE TO RESTART", LCD_GREEN, 2);
    }

    if (game_state != GAME_RUNNING) {
        if (centered) {
            start_armed = true;
        } else if (start_armed) {
            snake_init();
            snake_set_direction(direction);
            start_armed = false;
            draw_board();
            // Start the movement deadline after the blocking screen clear.
            snake_start();
            displayed_state = GAME_RUNNING;
        }
        return;
    }

    if (!centered) snake_set_direction(direction);
    int old_length = snake_get_length();
    snake_position_t old_tail = snake_get_segment(0);
    snake_position_t old_head = snake_get_segment(old_length - 1);
    snake_update();
    if (game_state != GAME_RUNNING) return;

    int length = snake_get_length();
    snake_position_t head = snake_get_segment(length - 1);
    if (head.x == old_head.x && head.y == old_head.y) return;

    // Update only changed cells so joystick polling and audio stay responsive.
    lcd_draw_cell(old_head.x, old_head.y, LCD_GREEN);
    if (length == old_length) lcd_draw_cell(old_tail.x, old_tail.y, LCD_BLACK);
    lcd_draw_cell(head.x, head.y, LCD_YELLOW);
    snake_position_t food = snake_get_food();
    lcd_draw_cell(food.x, food.y, LCD_RED);
}
