#include "audio.h"
#include "sev_seg_disp.h"
#include "game_state.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include "eeprom.h"
#include "uart.h"
#include "lcd.h"

#ifndef AUDIO_TEST_MODE
#define AUDIO_TEST_MODE 0
#endif

#ifndef LCD_TEST_MODE
#define LCD_TEST_MODE 1
#endif

game_state_t game_state = GAME_WAITING;
snake_direction_t game_requested_direction = DIRECTION_RIGHT;
uint32_t game_score = 0; //current/updating score
uint32_t high_score = 0; //added here for now, not sure if this is where it should stay yet
uint16_t game_joystick_x_raw = 0;
uint16_t game_joystick_y_raw = 0;
bool game_food_audio_event = false;
bool game_over_audio_event = false;

//these are for SPI/DMA to control the 7-segment display (used in sev_seg_disp.c)
const int SEG7_DMA_CHANNEL = 1;  // Set to the correct DMA channel for the 7-segment display for Step 4 autotest.

const int SPI_7SEG_SCK = 18; // Replace with your SCK pin number for the 7-segment display
const int SPI_7SEG_CSn = 17; // Replace with your CSn pin number for the 7-segment display
const int SPI_7SEG_TX = 19; // Replace with your TX pin number for the 7-segment display
//end of section

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

#if LCD_TEST_MODE

static void lcd_test_update(void)
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

#endif

int main(void)
{
    audio_init();
    // TODO: Joystick initialization
    // TODO: TFT initialization (
    // TODO: PIO score display initialization

    #if LCD_TEST_MODE
        lcd_init();
    #endif

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

        #if LCD_TEST_MODE
            lcd_test_update();
        #endif

        // TODO: TFT rendering
        // TODO: PIO score display update
        tight_loop_contents();
    }
}


//please dont delete, using for testing
/*
int main(void){
    //Test for seven-seg dipslay fuctionality

    // Configures our microcontroller to 
    // communicate over UART through the TX/RX pins
    stdio_init_all();
    high_score = 784;
    game_score = 576;
    prepare_scores();
    display_init_spi();
    display_init_dma();

    //if eat food
        //game_score++;
    //Want to code it so that the highscore only updates after death/completion of round
    //if die/game over
    //check condition 
    // if (game_score >= high_score){
    //     high_score = game_score
    // }

    //initialize high score in main.c, but update it in the EEPROM I2C thing
    //PRESS RESET ON MICROCONTROLLER TO SHOW THE PRINT
    init_uart();
     // You will construct your own main function.
    init_i2c();

    char hs_string[4] = {0}; //4 bc null terminator
    snprintf(hs_string, 4, "%d", high_score);

    eeprom_write(0x20, hs_string, 4);
    char data[33] = {0}; //was getting junk, changed to 33 so that there was space for null terminator and initialized to 0 for gernal safety
    eeprom_read(0x20, data, 32);

    printf("Data read from EEPROM: %s\n", data);
}
*/
