#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "stdio.h"
#include "game_state.h"

#define XAXIS //do I even need this?
#define YAXIS
// int x_val;
// int y_val;
int adc_x;
int adc_y;

void init_adc() {
    // fill in
    adc_init();
    adc_gpio_init(28); //whichever pin wired
    adc_select_input(2);
    adc_y = adc_get_selected_input();
    adc_gpio_init(27);
    adc_select_input(1);
    adc_x = adc_get_selected_input();
    //adc_irq_set_enabled(true);
}
/*
    Setting up round robbin so that it takes multiple inputs. 
    The problem is that this also needs to run continuously, hence why I'm doing this at the register level
*/
void init_adc_round_robin() {
    // fill in
    adc_irq_set_enabled(true);
    //adc_set_temp_sensor_enabled(true);
    init_adc();
    //adc_set_round_robin();

    uint mask = (0x6 << 16 || 0x6 << 12 || 1 << 3);
    adc_hw->cs; //0011
    // adc_select_input(5);
    // adc_run(true);

}

/*
    Needs to read the position of the snake while the game is running (probably uses snake_body)
    Wondering how to differentiate between directions in the context of the range we have
    Probably need to add an extra function to test all of this out
    What I plan to have show up in main will be the initializaion above and the read_joystick function 
    (while(true) loop will probably not be necessary, but I'll let you guys know when I actually get to that point)
*/
void read_joystick()
{
    //init_adc();
    
    game_joystick_x_raw = adc_read; 
    //will need to read adc_y and adc_x in continous loop, will need to change this because 
    //this sets up free run mode, which only reads one input
    game_joystick_y_raw = adc_read;

    //init_adc_freerun();
    //adc_set_round_robin;
    //24 collumns 32 rows --> snake.h
    while(GAME_RUNNING){
        
    }

    // int i = 0;
    // for(;;) {
    //     printf("ADC Result: %d     \r", adc_hw->result);
    //     fflush(stdout);
    //     sleep_ms(250);
    // }
}
