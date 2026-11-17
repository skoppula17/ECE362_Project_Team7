#include "audio.h"
#include "sev_seg_disp.h"
#include "game_state.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include "eeprom.h"
#include "uart.h"
#include "lcd.h"
#include "lcd_tests.h"
#include "snake.h"

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

const int SPI_7SEG_SCK = 10; // Replace with your SCK pin number for the 7-segment display
const int SPI_7SEG_CSn = 13; // Replace with your CSn pin number for the 7-segment display
const int SPI_7SEG_TX = 11; // Replace with your TX pin number for the 7-segment display
//end of section

// The scripted LCD demo uses snake.c, which raises these events on food
// and collision. Consume each event once; game over takes priority.
static void game_audio_update(void)
{
    if (game_over_audio_event) {
        game_over_audio_event = false;
        game_food_audio_event = false;
        audio_play_game_over();
    } else if (game_food_audio_event) {
        game_food_audio_event = false;
        audio_play_food();
    }
    audio_update();
}

int main(void)
{
    audio_init();
    // TODO: Joystick initialization
    stdio_init_all();
    display_init_spi();
    display_init_dma();

    #if LCD_TEST_MODE
        lcd_init();
    #endif

    while (true) {
        // TODO: Joystick update

        audio_update();

        #if LCD_TEST_MODE
            // lcd_test_update();                // Static graphics
            // lcd_movement_test_update();       // Straight movement
            // lcd_gameplay_test_update();       // Food and wall collisions
            // lcd_turning_test_update();        // Automatic turning
            //lcd_self_collision_test_update();    // Self-collision
            //lcd_snake_engine_test_update();  // Actual snake.c engine
            //lcd_snake_food_test_update();  // Food and growth, hits wall
            //lcd_snake_full_gameplay_test_update();
            //lcd_snake_final_collision_test_update();
            //lcd_start_screen_test_update();
            // Default demo: one buzz per food, three buzzes on final collision.
            // lcd_fill_rect() services audio while drawing the game-over screen.
            lcd_complete_game_test_update();
        #else
            // The joystick driver controls GAME_WAITING/GAME_RUNNING and direction.
            snake_update();
        #endif

        // Play the events produced by this iteration of the scripted demo/game.
        game_audio_update();

        if ((game_state == GAME_OVER) && (game_score >= high_score))
        {
            high_score = game_score;
        }
        // SPI1 DMA continuously refreshes the seven-segment display.
        // Only update its buffer here; display_print() writes to the LCD's SPI0.
        prepare_scores();
        tight_loop_contents();
    }

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
    if ((game_state == GAME_OVER) & (game_score >= high_score))
    {
        high_score = game_score;
    }
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
