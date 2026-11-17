
#include "snake.h"
#include "audio.h"
#include "pico/stdlib.h"
#include "pico/rand.h"

#define SNAKE_MOVE_INTERVAL_MS 250

static snake_position_t snake_body[SNAKE_MAX_LENGTH];
static int snake_length = 5;
static snake_position_t food;

static snake_direction_t current_direction = DIRECTION_RIGHT;
static absolute_time_t next_move;

static bool joystick_centered = true;
static bool game_initialized = false;

static bool directions_opposite(snake_direction_t a,
                                snake_direction_t b)
{
    return (a == DIRECTION_UP && b == DIRECTION_DOWN) ||
           (a == DIRECTION_DOWN && b == DIRECTION_UP) ||
           (a == DIRECTION_LEFT && b == DIRECTION_RIGHT) ||
           (a == DIRECTION_RIGHT && b == DIRECTION_LEFT);
}

static bool positions_equal(snake_position_t a,
                            snake_position_t b)
{
    return a.x == b.x && a.y == b.y;
}

static void snake_generate_food(void)
{
    if (snake_length >= SNAKE_MAX_LENGTH) {
        game_state = GAME_OVER;
        game_over_audio_event = true;
        return;
    }

    // Choose from empty cells directly, so a nearly full board cannot get
    // stuck repeatedly sampling occupied positions. Pico supplies the seed.
    bool occupied[SNAKE_MAX_LENGTH] = {false};
    for (int i = 0; i < snake_length; i++) {
        occupied[snake_body[i].y * SNAKE_GRID_WIDTH + snake_body[i].x] = true;
    }

    uint32_t empty_count = SNAKE_MAX_LENGTH - snake_length;
    uint32_t selected = get_rand_32() % empty_count;
    for (int cell = 0; cell < SNAKE_MAX_LENGTH; cell++) {
        if (occupied[cell]) {
            continue;
        }
        if (selected == 0) {
            food.x = cell % SNAKE_GRID_WIDTH;
            food.y = cell / SNAKE_GRID_WIDTH;
            return;
        }
        selected--;
    }
}

void snake_init(void)
{
    // audio_init() must run before initializing or restarting the game.
    audio_stop();
    snake_length = 5;
    game_score = 0;
    game_state = GAME_WAITING;
    current_direction = DIRECTION_RIGHT;
    game_requested_direction = DIRECTION_RIGHT;

    game_food_audio_event = false;
    game_over_audio_event = false;

    for (int i = 0; i < snake_length; i++) {
        snake_body[i].x = 5 + i;
        snake_body[i].y = 10;
    }

    snake_generate_food();

    joystick_centered = false;
    game_initialized = true;
}

void snake_set_direction(snake_direction_t direction)
{
    game_requested_direction = direction;
}

void snake_update(void)
{
    if (!game_initialized) {
        snake_init();
    }

    // Waiting and game-over transitions will be handled
    // after integrating joystick-center detection.
    if (game_state != GAME_RUNNING) {
        return;
    }

    if (!time_reached(next_move)) {
        return;
    }

    if (!directions_opposite(game_requested_direction,
                             current_direction)) {
        current_direction = game_requested_direction;
    }

    snake_position_t head = snake_body[snake_length - 1];
    snake_position_t new_head = head;

    switch (current_direction) {
    case DIRECTION_UP:
        new_head.y--;
        break;
    case DIRECTION_DOWN:
        new_head.y++;
        break;
    case DIRECTION_LEFT:
        new_head.x--;
        break;
    case DIRECTION_RIGHT:
        new_head.x++;
        break;
    }

    // Wall collision.
    if (new_head.x < 0 ||
        new_head.x >= SNAKE_GRID_WIDTH ||
        new_head.y < 0 ||
        new_head.y >= SNAKE_GRID_HEIGHT) {

        game_state = GAME_OVER;
        game_over_audio_event = true;
        return;
    }

    bool ate_food = positions_equal(new_head, food);

    // The tail moves away unless the snake eats food.
    int first_segment = ate_food ? 0 : 1;

    for (int i = first_segment; i < snake_length; i++) {
        if (positions_equal(new_head, snake_body[i])) {
            game_state = GAME_OVER;
            game_over_audio_event = true;
            return;
        }
    }

    if (ate_food && snake_length < SNAKE_MAX_LENGTH) {
        snake_body[snake_length] = new_head;
        snake_length++;

        game_score++;
        game_food_audio_event = true;

        snake_generate_food();
    } else {
        for (int i = 0; i < snake_length - 1; i++) {
            snake_body[i] = snake_body[i + 1];
        }

        snake_body[snake_length - 1] = new_head;

        if (ate_food) {
            game_score++;
            game_food_audio_event = true;
            snake_generate_food();
        }
    }

    next_move = make_timeout_time_ms(SNAKE_MOVE_INTERVAL_MS);
}

int snake_get_length(void)
{
    return snake_length;
}

snake_position_t snake_get_segment(int index)
{
    if (index < 0 || index >= snake_length) {
        return (snake_position_t){-1, -1};
    }

    return snake_body[index];
}

snake_position_t snake_get_food(void)
{
    return food;
}


bool snake_test_set_food(int x, int y)
{
    if (x < 0 || x >= SNAKE_GRID_WIDTH ||
        y < 0 || y >= SNAKE_GRID_HEIGHT) {
        return false;
    }

    // Food cannot overlap the snake.
    for (int i = 0; i < snake_length; i++) {
        if (snake_body[i].x == x &&
            snake_body[i].y == y) {
            return false;
        }
    }

    food.x = x;
    food.y = y;
    return true;
}

