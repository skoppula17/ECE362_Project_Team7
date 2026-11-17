#include "lcd_tests.h"
#include "lcd.h"
#include "game_state.h"
#include "pico/stdlib.h"
#include <stdbool.h>
#include "snake.h"
#include <stdio.h>

// Audio event flags are defined in main.c.
extern bool game_food_audio_event;
extern bool game_over_audio_event;

void lcd_test_update(void)
{
    static bool test_drawn = false;

    if (!test_drawn) {
        test_drawn = true;

        // Black background
        lcd_fill_screen(LCD_BLACK);

        // Snake body
        lcd_draw_cell(10, 10, LCD_GREEN);
        lcd_draw_cell(11, 10, LCD_GREEN);
        lcd_draw_cell(12, 10, LCD_GREEN);
        lcd_draw_cell(13, 10, LCD_GREEN);

        // Snake head
        lcd_draw_cell(14, 10, LCD_YELLOW);

        // Food
        lcd_draw_cell(18, 15, LCD_RED);
    }
}

void lcd_movement_test_update(void)
{
    static bool initialized = false;
    static int snake_x = 5;
    static absolute_time_t next_update;

    if (!initialized) {
        lcd_fill_screen(LCD_BLACK);

        // Draw initial snake
        for (int i = 0; i < 5; i++) {
            lcd_draw_cell(snake_x + i, 10, LCD_GREEN);
        }

        next_update = make_timeout_time_ms(250);
        initialized = true;
        return;
    }

    if (time_reached(next_update)) {
        // Erase the old tail
        lcd_draw_cell(snake_x, 10, LCD_BLACK);

        snake_x++;

        // Restart when approaching the right edge
        if (snake_x >= 19) {
            snake_x = 5;
            lcd_fill_screen(LCD_BLACK);

            for (int i = 0; i < 5; i++) {
                lcd_draw_cell(snake_x + i, 10, LCD_GREEN);
            }
        } else {
            // Draw new head
            lcd_draw_cell(snake_x + 4, 10, LCD_GREEN);
        }

        next_update = make_timeout_time_ms(250);
    }
}

void lcd_gameplay_test_update(void)
{
    #define TEST_SNAKE_MAX_LENGTH 32
    #define TEST_GRID_WIDTH 24
    #define TEST_GRID_HEIGHT 32

    static bool initialized = false;
    static bool game_over = false;

    static int snake_x[TEST_SNAKE_MAX_LENGTH];
    static int snake_y[TEST_SNAKE_MAX_LENGTH];
    static int snake_length = 5;

    static int food_x = 12;
    static int food_y = 10;

    static absolute_time_t next_update;

    if (!initialized) {
        lcd_fill_screen(LCD_BLACK);

        // Snake starts moving right
        for (int i = 0; i < snake_length; i++) {
            snake_x[i] = 5 + i;
            snake_y[i] = 10;
            lcd_draw_cell(snake_x[i], snake_y[i], LCD_GREEN);
        }

        // Yellow head
        lcd_draw_cell(snake_x[snake_length - 1],
                      snake_y[snake_length - 1], LCD_YELLOW);

        // Red food
        lcd_draw_cell(food_x, food_y, LCD_RED);

        next_update = make_timeout_time_ms(250);
        initialized = true;
        return;
    }

    // Stop movement after a collision
    if (game_over) {
        return;
    }

    if (!time_reached(next_update)) {
        return;
    }

    int head_x = snake_x[snake_length - 1];
    int head_y = snake_y[snake_length - 1];

    // Move right one cell
    int new_head_x = head_x + 1;
    int new_head_y = head_y;

    // Check wall collision
    if (new_head_x >= TEST_GRID_WIDTH) {
        game_over = true;
        game_over_audio_event = true;
        return;
    }

    // Check food collision
    bool ate_food =
        (new_head_x == food_x && new_head_y == food_y);

    if (ate_food) {
        game_food_audio_event = true;
    } else {
        // Erase tail if not growing
        lcd_draw_cell(snake_x[0], snake_y[0], LCD_BLACK);

        // Shift body forward
        for (int i = 0; i < snake_length - 1; i++) {
            snake_x[i] = snake_x[i + 1];
            snake_y[i] = snake_y[i + 1];
        }
    }

    // Add new head
    if (ate_food && snake_length < TEST_SNAKE_MAX_LENGTH) {
        snake_x[snake_length] = new_head_x;
        snake_y[snake_length] = new_head_y;
        snake_length++;
    } else {
        snake_x[snake_length - 1] = new_head_x;
        snake_y[snake_length - 1] = new_head_y;
    }

    // Previous head becomes green
    lcd_draw_cell(head_x, head_y, LCD_GREEN);

    // New head becomes yellow
    lcd_draw_cell(new_head_x, new_head_y, LCD_YELLOW);

    // Move food after eating
    if (ate_food) {
        food_x = 18;
        food_y = 15;
        lcd_draw_cell(food_x, food_y, LCD_RED);
    }

    next_update = make_timeout_time_ms(250);
}

void lcd_turning_test_update(void)
{
    #define TURN_SNAKE_MAX_LENGTH 32
    #define TURN_GRID_WIDTH 24
    #define TURN_GRID_HEIGHT 32

    static bool initialized = false;
    static bool game_over = false;

    static int snake_x[TURN_SNAKE_MAX_LENGTH];
    static int snake_y[TURN_SNAKE_MAX_LENGTH];
    static int snake_length = 5;

    static int direction = 0; // 0=right, 1=down, 2=left, 3=up
    static int move_count = 0;

    static absolute_time_t next_update;

    if (!initialized) {
        lcd_fill_screen(LCD_BLACK);

        // Start with a horizontal snake
        for (int i = 0; i < snake_length; i++) {
            snake_x[i] = 5 + i;
            snake_y[i] = 10;
            lcd_draw_cell(snake_x[i], snake_y[i], LCD_GREEN);
        }

        lcd_draw_cell(snake_x[snake_length - 1],
                      snake_y[snake_length - 1], LCD_YELLOW);

        next_update = make_timeout_time_ms(250);
        initialized = true;
        return;
    }

    if (game_over || !time_reached(next_update)) {
        return;
    }

    // Automatically turn every 5 moves
    if (move_count > 0 && move_count % 5 == 0) {
        direction = (direction + 1) % 4;
    }

    int dx = 0;
    int dy = 0;

    switch (direction) {
    case 0: dx = 1;  break; // Right
    case 1: dy = 1;  break; // Down
    case 2: dx = -1; break; // Left
    case 3: dy = -1; break; // Up
    }

    int head_x = snake_x[snake_length - 1];
    int head_y = snake_y[snake_length - 1];

    int new_head_x = head_x + dx;
    int new_head_y = head_y + dy;

    // Check wall collisions
    if (new_head_x < 0 || new_head_x >= TURN_GRID_WIDTH ||
        new_head_y < 0 || new_head_y >= TURN_GRID_HEIGHT) {
        game_over = true;
        game_over_audio_event = true;
        return;
    }

    // Check self-collisions
    // Exclude the tail because it moves away on this step
    for (int i = 1; i < snake_length; i++) {
        if (snake_x[i] == new_head_x &&
            snake_y[i] == new_head_y) {
            game_over = true;
            game_over_audio_event = true;
            return;
        }
    }

    // Erase old tail
    lcd_draw_cell(snake_x[0], snake_y[0], LCD_BLACK);

    // Shift body positions
    for (int i = 0; i < snake_length - 1; i++) {
        snake_x[i] = snake_x[i + 1];
        snake_y[i] = snake_y[i + 1];
    }

    snake_x[snake_length - 1] = new_head_x;
    snake_y[snake_length - 1] = new_head_y;

    // Old head becomes green
    lcd_draw_cell(head_x, head_y, LCD_GREEN);

    // New head becomes yellow
    lcd_draw_cell(new_head_x, new_head_y, LCD_YELLOW);

    move_count++;
    next_update = make_timeout_time_ms(250);
}


void lcd_self_collision_test_update(void)
{
    #define SELF_TEST_LENGTH 10
    #define SELF_TEST_GRID_WIDTH 24
    #define SELF_TEST_GRID_HEIGHT 32

    static bool initialized = false;
    static bool game_over = false;

    static int snake_x[SELF_TEST_LENGTH];
    static int snake_y[SELF_TEST_LENGTH];

    static int move_count = 0;
    static absolute_time_t next_update;

    if (!initialized) {
        lcd_fill_screen(LCD_BLACK);

        // Start with a 10-cell snake moving right.
        for (int i = 0; i < SELF_TEST_LENGTH; i++) {
            snake_x[i] = i;
            snake_y[i] = 10;

            lcd_draw_cell(snake_x[i], snake_y[i], LCD_GREEN);
        }

        // Draw the head yellow.
        lcd_draw_cell(snake_x[SELF_TEST_LENGTH - 1],
                      snake_y[SELF_TEST_LENGTH - 1],
                      LCD_YELLOW);

        next_update = make_timeout_time_ms(350);
        initialized = true;
        return;
    }

    // Stop updating after game over.
    if (game_over || !time_reached(next_update)) {
        return;
    }

    // Deliberately follow a tight rectangular path:
    // Right 2, Down 2, Left 2, Up 2.
    // The last upward move collides with the body.
    int dx = 0;
    int dy = 0;

    if (move_count < 2) {
        dx = 1;   // Right
    } else if (move_count < 4) {
        dy = 1;   // Down
    } else if (move_count < 6) {
        dx = -1;  // Left
    } else {
        dy = -1;  // Up
    }

    int head_x = snake_x[SELF_TEST_LENGTH - 1];
    int head_y = snake_y[SELF_TEST_LENGTH - 1];

    int new_head_x = head_x + dx;
    int new_head_y = head_y + dy;

    // Check wall collision.
    if (new_head_x < 0 ||
        new_head_x >= SELF_TEST_GRID_WIDTH ||
        new_head_y < 0 ||
        new_head_y >= SELF_TEST_GRID_HEIGHT) {

        game_over = true;
        game_over_audio_event = true;
        return;
    }

    // Check self-collision.
    // Exclude the tail because it moves away this turn.
    for (int i = 1; i < SELF_TEST_LENGTH; i++) {
        if (snake_x[i] == new_head_x &&
            snake_y[i] == new_head_y) {

            game_over = true;
            game_over_audio_event = true;

            // Mark the collision location red.
            lcd_draw_cell(new_head_x, new_head_y, LCD_RED);
            return;
        }
    }

    // Erase the old tail.
    lcd_draw_cell(snake_x[0], snake_y[0], LCD_BLACK);

    // Shift each body segment forward.
    for (int i = 0; i < SELF_TEST_LENGTH - 1; i++) {
        snake_x[i] = snake_x[i + 1];
        snake_y[i] = snake_y[i + 1];
    }

    // Update the head position.
    snake_x[SELF_TEST_LENGTH - 1] = new_head_x;
    snake_y[SELF_TEST_LENGTH - 1] = new_head_y;

    // Previous head becomes green.
    lcd_draw_cell(head_x, head_y, LCD_GREEN);

    // New head becomes yellow.
    lcd_draw_cell(new_head_x, new_head_y, LCD_YELLOW);

    move_count++;
    next_update = make_timeout_time_ms(350);
}



void lcd_snake_engine_test_update(void)
{
    static bool initialized = false;
    static int move_count = 0;
    static snake_position_t previous_body[SNAKE_MAX_LENGTH];
    static int previous_length = 0;
    static snake_position_t previous_food;

    if (!initialized) {
        snake_init();
        game_state = GAME_RUNNING;

        lcd_fill_screen(LCD_BLACK);

        previous_food = snake_get_food();
        lcd_draw_cell(previous_food.x, previous_food.y, LCD_RED);

        previous_length = snake_get_length();

        for (int i = 0; i < previous_length; i++) {
            previous_body[i] = snake_get_segment(i);
            lcd_draw_cell(previous_body[i].x,
                          previous_body[i].y,
                          i == previous_length - 1 ?
                          LCD_YELLOW : LCD_GREEN);
        }

        initialized = true;
        return;
    }

    if (game_state != GAME_RUNNING) {
        return;
    }

    // Automatically request turns based on movement count.
    // This creates a square path.
    switch ((move_count / 5) % 4) {
    case 0:
        snake_set_direction(DIRECTION_RIGHT);
        break;
    case 1:
        snake_set_direction(DIRECTION_DOWN);
        break;
    case 2:
        snake_set_direction(DIRECTION_LEFT);
        break;
    case 3:
        snake_set_direction(DIRECTION_UP);
        break;
    }

    // Save the old head position.
    snake_position_t old_head =
        snake_get_segment(snake_get_length() - 1);

    // Update the actual game engine.
    snake_update();

    // Check whether the snake actually moved.
    snake_position_t new_head =
        snake_get_segment(snake_get_length() - 1);

    if (old_head.x == new_head.x &&
        old_head.y == new_head.y) {
        return;
    }

    // Erase all previous snake cells.
    for (int i = 0; i < previous_length; i++) {
        lcd_draw_cell(previous_body[i].x,
                      previous_body[i].y,
                      LCD_BLACK);
    }

    // Draw the new food position.
    snake_position_t food = snake_get_food();
    lcd_draw_cell(food.x, food.y, LCD_RED);

    // Draw the updated snake.
    int length = snake_get_length();

    for (int i = 0; i < length; i++) {
        snake_position_t segment = snake_get_segment(i);

        lcd_draw_cell(segment.x,
                      segment.y,
                      i == length - 1 ?
                      LCD_YELLOW : LCD_GREEN);

        previous_body[i] = segment;
    }

    previous_length = length;
    previous_food = food;
    move_count++;
}


void lcd_snake_food_test_update(void)
{
    static bool initialized = false;
    static snake_position_t previous_body[SNAKE_MAX_LENGTH];
    static int previous_length = 0;
    static uint32_t previous_score = 0;

    if (!initialized) {
        snake_init();

        // Start the game without joystick input.
        game_state = GAME_RUNNING;
        snake_set_direction(DIRECTION_RIGHT);

        // Initial head is at (9, 10).
        // Place food directly in its path.
        snake_test_set_food(12, 10);

        lcd_fill_screen(LCD_BLACK);

        snake_position_t food = snake_get_food();
        lcd_draw_cell(food.x, food.y, LCD_RED);

        previous_length = snake_get_length();

        for (int i = 0; i < previous_length; i++) {
            previous_body[i] = snake_get_segment(i);

            lcd_draw_cell(previous_body[i].x,
                          previous_body[i].y,
                          i == previous_length - 1 ?
                          LCD_YELLOW : LCD_GREEN);
        }

        previous_score = game_score;
        initialized = true;
        return;
    }

    if (game_state != GAME_RUNNING) {
        return;
    }

    snake_position_t old_head =
        snake_get_segment(snake_get_length() - 1);

    snake_update();

    snake_position_t new_head =
        snake_get_segment(snake_get_length() - 1);

    // Only redraw when the snake actually moves.
    if (old_head.x == new_head.x &&
        old_head.y == new_head.y) {
        return;
    }

    // Erase previous snake.
    for (int i = 0; i < previous_length; i++) {
        lcd_draw_cell(previous_body[i].x,
                      previous_body[i].y,
                      LCD_BLACK);
    }

    // Draw food at its current position.
    snake_position_t food = snake_get_food();
    lcd_draw_cell(food.x, food.y, LCD_RED);

    // Draw updated snake.
    int length = snake_get_length();

    for (int i = 0; i < length; i++) {
        snake_position_t segment = snake_get_segment(i);

        lcd_draw_cell(segment.x,
                      segment.y,
                      i == length - 1 ?
                      LCD_YELLOW : LCD_GREEN);

        previous_body[i] = segment;
    }

    previous_length = length;

    // Check that the score increased.
    if (game_score > previous_score) {
        previous_score = game_score;
    }
}


void lcd_snake_full_gameplay_test_update(void)
{
    static bool initialized = false;
    static int move_count = 0;
    static int food_count = 0;

    static snake_position_t previous_body[SNAKE_MAX_LENGTH];
    static int previous_length = 0;

    if (!initialized) {
        snake_init();
        game_state = GAME_RUNNING;

        // Start by moving right.
        snake_set_direction(DIRECTION_RIGHT);

        // Place first food in front of the snake.
        snake_test_set_food(12, 10);

        lcd_fill_screen(LCD_BLACK);

        previous_length = snake_get_length();

        for (int i = 0; i < previous_length; i++) {
            previous_body[i] = snake_get_segment(i);
            lcd_draw_cell(previous_body[i].x,
                          previous_body[i].y,
                          i == previous_length - 1 ?
                          LCD_YELLOW : LCD_GREEN);
        }

        snake_position_t food = snake_get_food();
        lcd_draw_cell(food.x, food.y, LCD_RED);

        initialized = true;
        return;
    }

    if (game_state != GAME_RUNNING) {
        return;
    }

    // Follow a planned path.
    if (move_count < 5) {
        snake_set_direction(DIRECTION_RIGHT);
    } else if (move_count < 9) {
        snake_set_direction(DIRECTION_DOWN);
    } else if (move_count < 13) {
        snake_set_direction(DIRECTION_LEFT);
    } else {
        snake_set_direction(DIRECTION_UP);
    }

    snake_position_t old_head =
        snake_get_segment(snake_get_length() - 1);

    uint32_t old_score = game_score;

    snake_update();

    snake_position_t new_head =
        snake_get_segment(snake_get_length() - 1);

    if (old_head.x == new_head.x &&
        old_head.y == new_head.y) {
        return;
    }

    move_count++;

    // Place the next food after each successful pickup.
    if (game_score > old_score) {
        food_count++;

        if (food_count == 1) {
            snake_test_set_food(14, 13);
        } else if (food_count == 2) {
            snake_test_set_food(11, 14);
        }
    }

    // Erase old snake cells.
    for (int i = 0; i < previous_length; i++) {
        lcd_draw_cell(previous_body[i].x,
                      previous_body[i].y,
                      LCD_BLACK);
    }

    // Draw food.
    snake_position_t food = snake_get_food();
    lcd_draw_cell(food.x, food.y, LCD_RED);

    // Draw updated snake.
    int length = snake_get_length();

    for (int i = 0; i < length; i++) {
        snake_position_t segment = snake_get_segment(i);

        lcd_draw_cell(segment.x,
                      segment.y,
                      i == length - 1 ?
                      LCD_YELLOW : LCD_GREEN);

        previous_body[i] = segment;
    }

    previous_length = length;
}





void lcd_snake_final_collision_test_update(void)
{
    static bool initialized = false;
    static int move_count = 0;

    static snake_position_t previous_body[SNAKE_MAX_LENGTH];
    static int previous_length = 0;
    static snake_position_t previous_food;

    if (!initialized) {
        snake_init();
        game_state = GAME_RUNNING;

        move_count = 0;
        snake_set_direction(DIRECTION_RIGHT);

        // First food: eaten on movement 3.
        snake_test_set_food(12, 10);

        lcd_fill_screen(LCD_BLACK);

        previous_length = snake_get_length();

        for (int i = 0; i < previous_length; i++) {
            previous_body[i] = snake_get_segment(i);

            lcd_draw_cell(previous_body[i].x,
                          previous_body[i].y,
                          i == previous_length - 1 ?
                          LCD_YELLOW : LCD_GREEN);
        }

        previous_food = snake_get_food();

        lcd_draw_cell(previous_food.x,
                      previous_food.y,
                      LCD_RED);

        initialized = true;
        return;
    }

    if (game_state != GAME_RUNNING) {
        return;
    }

    // Long route:
    // Right 8, Down 6, Left 7, Up 5,
    // Right 4, Down 2, Left 2,
    // Up 1, Right 1, Down 1.
    //
    // Final move collides with the snake's body.

    if (move_count < 8) {
        snake_set_direction(DIRECTION_RIGHT);
    } else if (move_count < 14) {
        snake_set_direction(DIRECTION_DOWN);
    } else if (move_count < 21) {
        snake_set_direction(DIRECTION_LEFT);
    } else if (move_count < 26) {
        snake_set_direction(DIRECTION_UP);
    } else if (move_count < 30) {
        snake_set_direction(DIRECTION_RIGHT);
    } else if (move_count < 32) {
        snake_set_direction(DIRECTION_DOWN);
    } else if (move_count < 34) {
        snake_set_direction(DIRECTION_LEFT);
    } else if (move_count < 35) {
        snake_set_direction(DIRECTION_UP);
    } else if (move_count < 36) {
        snake_set_direction(DIRECTION_RIGHT);
    } else {
        snake_set_direction(DIRECTION_DOWN);
    }

    snake_position_t old_head =
        snake_get_segment(snake_get_length() - 1);

    uint32_t old_score = game_score;

    snake_update();

    // Stop after the planned self-collision.
    if (game_state == GAME_OVER) {
        lcd_draw_cell(old_head.x,
                      old_head.y,
                      LCD_RED);
        return;
    }

    snake_position_t new_head =
        snake_get_segment(snake_get_length() - 1);

    // Only count completed movements.
    if (old_head.x == new_head.x &&
        old_head.y == new_head.y) {
        return;
    }

    move_count++;

    // Place the next food along the new route.
    if (game_score > old_score) {
        bool placed = true;

        if (game_score == 1) {
            // Second food: movement 11.
            placed = snake_test_set_food(17, 13);
        } else if (game_score == 2) {
            // Third food: movement 18.
            placed = snake_test_set_food(13, 16);
        }

        // Stop the test if deterministic placement fails.
        if (!placed) {
            game_state = GAME_OVER;
            game_over_audio_event = true;
            return;
        }
    }

    // Erase the previous snake.
    for (int i = 0; i < previous_length; i++) {
        lcd_draw_cell(previous_body[i].x,
                      previous_body[i].y,
                      LCD_BLACK);
    }

    // Erase the previous food.
    lcd_draw_cell(previous_food.x,
                  previous_food.y,
                  LCD_BLACK);

    // Draw the current food.
    snake_position_t food = snake_get_food();

    lcd_draw_cell(food.x,
                  food.y,
                  LCD_RED);

    previous_food = food;

    // Draw the updated snake.
    int length = snake_get_length();

    for (int i = 0; i < length; i++) {
        snake_position_t segment = snake_get_segment(i);

        previous_body[i] = segment;

        lcd_draw_cell(segment.x,
                      segment.y,
                      i == length - 1 ?
                      LCD_YELLOW : LCD_GREEN);
    }

    previous_length = length;
}



void lcd_start_screen_test_update(void)
{
    static bool initialized = false;

    if (initialized) {
        return;
    }

    lcd_fill_screen(LCD_BLACK);

    lcd_draw_text(30, 65, "SNAKE BYTE",
                  LCD_GREEN, 3);

    lcd_draw_text(48, 155, "MOVE JOYSTICK",
                  LCD_WHITE, 2);

    lcd_draw_text(12, 180, "IN ANY DIRECTION",
                  LCD_WHITE, 2);

    lcd_draw_text(78, 205, "TO START",
                  LCD_YELLOW, 2);

    initialized = true;
}



void lcd_game_over_screen_test(void)
{
    // Clear the screen.
    lcd_fill_screen(LCD_BLACK);

    // Display GAME OVER.
    lcd_draw_text(39, 80, "GAME OVER",
                  LCD_RED, 3);

    // Display the actual score from snake.c.
    char score_text[24];

    snprintf(score_text, sizeof(score_text),
             "SCORE %lu", (unsigned long)game_score);

    lcd_draw_text(48, 150, score_text,
                  LCD_WHITE, 2);

    // Display restart instructions.
    lcd_draw_text(12, 210, "PRESS RESET TO",
                  LCD_GREEN, 2);

    lcd_draw_text(12, 235, "RESTART GAME",
                  LCD_GREEN, 2);
}



void lcd_complete_game_test_update(void)
{
    static int demo_state = 0;
    static absolute_time_t start_time;

    switch (demo_state) {
        case 0:
            lcd_start_screen_test_update();
            start_time = make_timeout_time_ms(3000);
            demo_state = 1;
            break;

        case 1:
            if (time_reached(start_time)) {
                demo_state = 2;
            }
            break;

        case 2:
            lcd_snake_final_collision_test_update();

            if (game_state == GAME_OVER) {
                demo_state = 3;
            }
            break;

        case 3:
            lcd_game_over_screen_test();
            demo_state = 4;
            break;

        case 4:
            // Keep game-over screen displayed.
            break;
    }
}


void lcd_snake_wall_collision_test_update(void)
{
    static bool initialized = false;
    static int move_count = 0;

    static snake_position_t previous_body[SNAKE_MAX_LENGTH];
    static int previous_length = 0;
    static snake_position_t previous_food;

    if (!initialized) {
        snake_init();
        game_state = GAME_RUNNING;

        move_count = 0;
        snake_set_direction(DIRECTION_RIGHT);

        // First food: movement 3.
        snake_test_set_food(12, 10);

        lcd_fill_screen(LCD_BLACK);

        previous_length = snake_get_length();

        for (int i = 0; i < previous_length; i++) {
            previous_body[i] = snake_get_segment(i);

            lcd_draw_cell(previous_body[i].x,
                          previous_body[i].y,
                          i == previous_length - 1 ?
                          LCD_YELLOW : LCD_GREEN);
        }

        previous_food = snake_get_food();

        lcd_draw_cell(previous_food.x,
                      previous_food.y,
                      LCD_RED);

        initialized = true;
        return;
    }

    if (game_state != GAME_RUNNING) {
        return;
    }

    // Move right until reaching (23, 10).
    // Then move down until hitting the bottom wall.
    if (move_count < 14) {
        snake_set_direction(DIRECTION_RIGHT);
    } else {
        snake_set_direction(DIRECTION_DOWN);
    }

    snake_position_t old_head =
        snake_get_segment(snake_get_length() - 1);

    uint32_t old_score = game_score;

    snake_update();

    // Stop after wall collision.
    if (game_state == GAME_OVER) {
        lcd_draw_cell(old_head.x,
                      old_head.y,
                      LCD_RED);
        return;
    }

    snake_position_t new_head =
        snake_get_segment(snake_get_length() - 1);

    // Only count completed movements.
    if (old_head.x == new_head.x &&
        old_head.y == new_head.y) {
        return;
    }

    move_count++;

    // Place the next food along the route.
    if (game_score > old_score) {
        bool placed = true;

        if (game_score == 1) {
            placed = snake_test_set_food(15, 10);
        } else if (game_score == 2) {
            placed = snake_test_set_food(18, 10);
        } else if (game_score == 3) {
            placed = snake_test_set_food(21, 10);
        } else if (game_score == 4) {
            placed = snake_test_set_food(23, 13);
        }

        if (!placed) {
            game_state = GAME_OVER;
            game_over_audio_event = true;
            return;
        }
    }

    // Erase the previous snake.
    for (int i = 0; i < previous_length; i++) {
        lcd_draw_cell(previous_body[i].x,
                      previous_body[i].y,
                      LCD_BLACK);
    }

    // Erase the previous food.
    lcd_draw_cell(previous_food.x,
                  previous_food.y,
                  LCD_BLACK);

    // Draw the current food.
    snake_position_t food = snake_get_food();

    lcd_draw_cell(food.x,
                  food.y,
                  LCD_RED);

    previous_food = food;

    // Draw the updated snake.
    int length = snake_get_length();

    for (int i = 0; i < length; i++) {
        snake_position_t segment = snake_get_segment(i);

        previous_body[i] = segment;

        lcd_draw_cell(segment.x,
                      segment.y,
                      i == length - 1 ?
                      LCD_YELLOW : LCD_GREEN);
    }

    previous_length = length;
}

void lcd_complete_wall_game_test_update(void)
{
    static int demo_state = 0;
    static absolute_time_t start_time;

    switch (demo_state) {
        case 0:
            lcd_start_screen_test_update();
            start_time = make_timeout_time_ms(3000);
            demo_state = 1;
            break;

        case 1:
            if (time_reached(start_time)) {
                demo_state = 2;
            }
            break;

        case 2:
            lcd_snake_wall_collision_test_update();

            if (game_state == GAME_OVER) {
                demo_state = 3;
            }
            break;

        case 3:
            lcd_game_over_screen_test();
            demo_state = 4;
            break;

        case 4:
            // Keep game-over screen displayed.
            break;
    }
}