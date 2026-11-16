//this is fro the I2C and EEPROM. copied it over from my past lab and the optional I2C lab
#include "hardware/uart.h" 
#include "hardware/gpio.h" 

// copy in defns for BUFSIZE, serbuf, seridx, newline_seen
#define BUFSIZE 32
char serbuf[BUFSIZE];
int seridx = 0;
int newline_seen = 0;

// from UART lab, as well as functions below.

void init_uart() {
    // fill in
    // Set the GPIO pin mux to the UART - pin 0 is TX, 1 is RX; note use of UART_FUNCSEL_NUM 
    gpio_set_function(0, UART_FUNCSEL_NUM(uart0, 0));
    gpio_set_function(1, UART_FUNCSEL_NUM(uart0, 1));

    // Initialise UART 0
    uart_init(uart0, 115200);
    uart0_hw->lcr_h = (0x3 << 5) | (0 << 1) | (0 << 3);
    // b11 = 8 bits, so hex 0x3, hence (0x3 << 5)
    //(0 << 1), ensures parity is disabled
    //(0 << 3), ensures 1 stop bit, not 2
}

// add this here so that compiler does not complain about implicit function
void uart_rx_handler();

void init_uart_irq() {
    // fill in
    //fifo is already disabled, never told to enable it

    //Enable the necessary bits to mask the receive interrupt
    uart0_hw->imsc = (1 << 4);//UARTIMSC register (datasheet)

    //set up exclusive handler
    irq_set_exclusive_handler(UART0_IRQ, uart_rx_handler);

    //enable interrupt
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irqs_enabled(uart0, 1, 0);
}

void uart_rx_handler() {
    // fill in
    //acknowledge interrupt
    //execute clear recieve interrupt (bit 4)
    uart0_hw->icr = (1 << 4); //UARTICR Register

    //If seridx reaches BUFSIZE, return immediately
    if (seridx == BUFSIZE){
        return;
    }

    //it said no sdk function, this is my best guess
    char c = uart0_hw->dr; 
    //if c is a newline, update newline_seen
    if (c == 0x0A){
        newline_seen = 1;
    }
    //if c is a backspace
    if (c == 0x8){
        uart_putc(uart0, 0x8); //write backspace
        uart_putc(uart0, 0x20); //write space (ASCII 0x20 or 32)
        uart_putc(uart0, 0x8); //write another backspace
        //
        if (seridx){
            seridx --;
            serbuf[seridx] = '\0';
            return;
        }
    }
    uart_putc(uart0, c); //print out with uart
    serbuf[seridx] = c; //write to buffer
    seridx++; //increment
}


int _read(__unused int handle, char *buffer, int length) {
    // fill in
    //Until a newline is seen, sleep 5 milliseconds.
    while (!newline_seen){
        sleep_ms(5);
    }
    newline_seen = 0;
    int temp = 0;
    (length >= seridx) ? (temp = seridx) : (temp = length);
    for (int i = 0; i < temp; i++){
        buffer[i] = serbuf[i];    
    }
    seridx = 0;
    return (temp);
}

int _write(__unused int handle, char *buffer, int length) {
    // fill in
    for (int i = 0; i < length; i++){
       uart_putc(uart0, buffer[i]);
    }
    return (length);
}