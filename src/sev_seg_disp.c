#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "get_sev_seg_val.h"
#include "math.h"
#include "stdio.h"


uint16_t __attribute__((aligned(16))) msg[8] = {
    (0 << 8) | 0xF6, // seven-segment value of 0
    0,
    0,
    0,
    //(1 << 8) | 0x06, // seven-segment value of 1
    //(2 << 8) | 0x5B, // seven-segment value of 2
    //(3 << 8) | 0x4F, // seven-segment value of 3
    (4 << 8) | 0xB9, // seven-segment value of 4
    0,
    0,
    0,
    //(5 << 8) | 0x6D, // seven-segment value of 5
    //(6 << 8) | 0x7D, // seven-segment value of 6
    //(7 << 8) | 0x07, // seven-segment value of 7
};

//font uses the table in the font.S file
extern char font[]; // Font mapping for 7-segment display
extern const int SPI_7SEG_SCK; extern const int SPI_7SEG_CSn; extern const int SPI_7SEG_TX;
static int index = 0; // Current index in the message buffer
//these are being pulled from main
extern uint32_t game_score; //current/updating score
extern uint32_t high_score; //high score

void prepare_scores() {
    //takes each digit in the score variables and assigns them to new variables to be used in the meassage array
    uint16_t temp = 0;
    uint16_t idx = 3; //3 then 2 then 1
    for (int i = 0; i < 3; i++){
        temp = (high_score / (int)(1 * pow(10,i))) % 10; //using int division by orders of magnitude to get the value of the 1s place, then 10s place, then 100s place
        //temp = (high_score >> (i * 4)) & 0xF;
        temp = get_sev_seg_val(temp);
        msg[idx] = (idx << 8) | temp;
        idx--;
    }

    uint16_t temp2 = 0;
    uint16_t idx2 = 7; //7 then 6 then 5
    for (int j = 0; j < 3; j++){
        temp2 = (game_score / (int)(1 * pow(10,j))) % 10;;
        temp2 = get_sev_seg_val(temp2);
        msg[idx2] = (idx2 << 8) | temp2;
        idx2--;
    }
}

void display_init_spi() {
    // fill in  
    //initialize the three GPIO pins to be used by the SPI peripheral associated with them
    gpio_set_function(SPI_7SEG_CSn, GPIO_FUNC_SPI);
    gpio_set_function(SPI_7SEG_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_7SEG_TX, GPIO_FUNC_SPI);

    //initialize spi
    spi_init(spi1, 125000); //baud rate = 125kHZ

    //set up spi format
    spi_set_format(spi1, 16, 0, 0, SPI_MSB_FIRST);  
}

void display_print() {
    // fill in    

    //write to SPI
    spi_write16_blocking(spi0, &msg[index], 1);

    //update index of msg
    index = (index + 1) % 8;
}

void display_init_dma() {
    // fill in    

    //using DMA channel 1 bc lab 4 uses channel 0

    //
    dma_hw->ch[1].read_addr = &msg; //source address
    dma_hw->ch[1].write_addr = &spi_get_hw(spi1)->dr; //destination address, spi1 data register is sspdr which corresponds to dr

    //count 8 (0x8 in hex), self trigger (shift a 1 to 28)
    dma_hw->ch[1].transfer_count = (0x8 << 0) | (1 << 28);

    (dma_hw->ch[1].ctrl_trig) = 0; //initialize to 0

    //coming from ctrl_trig_registers, pg 1128 of datasheet
    //data_size = 1; //need a 16 bit (half word which is 1) at bit 2 of ctrl_trig/al1_ctrl
    //Select SPI1 TX as the Data Request (DREQ) source
    //treq_sel = 0x30; //need 26 (0x1A) over bits 22-17 (based on spi1 usage)
    //enable the DMA channel
    //en = 1; //need a 1 at bit 0
    //The read address should increment after every transfer.
    //incr_read = 1; //need 1 at bit 4
    //The read address should wrap every X bits of the address. (Figure out what X is.)
    //X = 4; 2^4, x is power. 2^4 = 16. wrapping uses the power needed to achieve a certain number of bits 
    //ring_sel = 0; //no shift needed
    //ring_size = X; need X over bits 8-11
    
    uint32_t temp = (1 << 2) | (0x1A << 17) | (1 << 4) | (4 << 8) | (1 << 0); //temp variable

    (dma_hw->ch[1].ctrl_trig) |= temp; // or equals to be safe


}

// We provide you with this function for directly displaying characters.
// This accounts for the decimal point in the 7-segment display, as well as
// the SPI/DMA-friendly format of the message buffer.
/*
void display_char_print(const char message[]) {
    int dp_found = 0; 
    int out_idx = 0;
    for (int i = 0; i < 8 && message[i] != '\0'; i++) {
        if (message[i] == '.') {
            if (dp_found) {
                continue; // Ignore additional decimal points
            }
            if (out_idx > 0) {
                msg[out_idx - 1] |= (1 << 7); // Set decimal point bit
            }
            dp_found = 1;
        } else {
            uint16_t seg = font[(unsigned char)message[i]];
            // Insert position bits at bits 8-10
            seg |= (out_idx & 0x7) << 8;
            msg[out_idx] = seg;
            out_idx++;
        }
    }
    // Clear remaining digits if message shorter than 8
    for (; out_idx < 8; out_idx++) {
        msg[out_idx] = (out_idx << 8); // Only position bits, blank char
    }
}
*/