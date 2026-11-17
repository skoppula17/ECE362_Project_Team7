#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include <math.h>
#include <string.h>
#include "hardware/i2c.h"

#define I2C_SDA 30 //GPIO pin I chose
#define I2C_SCL 31 //GPIO pin I chose


// 0x50 requires that A2-A0 pins are all 0. (wired to ground)
#define EEPROM_ADDR 0x50

void init_i2c() {
    //initialize gpio pics to work with I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    //initialize I2C function
    i2c_init(i2c1, 400000); //400kHz for fast mode (RP datasheet pg 894 "12.2.3. I2C overview")
}


void eeprom_write(uint16_t loc, const char* data, uint8_t len) {
    uint8_t arr[32 + 2] = {0}; //32 bc max size of len
    arr[0] = (loc >> 8); //upper 8 bits of loc
    arr[1] = loc & 0xFF; //lower 8 bits of loc
    memcpy(&arr[2], data, len);// loc goes right after the address of data
    i2c_write_blocking(i2c1, 0x50, arr, (len + 2), false);

    sleep_ms(5); //wouldn't get the "Hello, EEPROM" without this for some reason
}

void eeprom_read(uint16_t loc, char data[], uint8_t len) {
    //PICO_ERROR_GENERIC;
    uint8_t arr[2] = {0}; //32 bc max size of len
    arr[0] = (loc >> 8); //upper 8 bits of loc
    arr[1] = loc & 0xFF; //lower 8 bits of loc

    int success_check = i2c_write_blocking(i2c1, 0x50, arr, 2, true);
    if (success_check > 0){
        i2c_read_blocking(i2c1, 0x50, data, len, false);

    }
    
}

//////////////////////////////////////////////////////////////////////////////

// Provided for you.

void write(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: write <addr> <data>\n");
        printf("Ensure the address is a hexadecimal number.  No need to include 0x.\n");
        return;
    }
    uint32_t addr = strtol(argv[1], NULL, 16); 
    // concatenate all args from argv[2], until empty string is found, to a string
    char data[32] = "";
    int i = 0;
    int j = 2;
    // Concatenate all args from argv[2] onwards into data, separated by spaces
    for (j = 2; j < argc && i < 32; j++) {
        int arg_len = strlen(argv[j]);
        int copy_len = (i + arg_len < 32) ? arg_len : (32 - i);
        strncpy(&data[i], argv[j], copy_len);
        i += copy_len;
        if (j < argc - 1 && i < 32) {
            data[i++] = ' ';
        }
    }
    data[i] = '\0';
    // ensure addr is a multiple of 32
    if ((addr % 32) != 0) {
        printf("Address 0x%lx is not evenly divisible by 32 (0x20).  Your address must be a hexadecimal value.\n", addr);
        return;
    }
    int msglen = strlen(data);
    if (msglen > 32) {
        printf("Data is too long. Max length is 32.\n");
        return;
    }
    else {
        // pad with spaces so it overwrites the entire 32 bytes
        for (int k = msglen; k < 31; k++) {
            data[k] = ' ';
        }
        data[31] = '\0';
    }
    printf("Writing to address 0x%lx: %s\n", addr, data);
    eeprom_write(addr, data, 32);
}

void read(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: read <addr>\n");
        printf("Ensure the address is a hexadecimal number.  No need to include 0x.\n");
        return;
    }
    uint32_t addr = strtol(argv[1], NULL, 16); 
    char data[32];
    // ensure addr is a multiple of 32
    if ((addr % 32) != 0) {
        printf("Address 0x%lx is not evenly divisible by 32 (0x20).  Your address must be a hexadecimal value.\n", addr);
        return;
    }
    eeprom_read(addr, data, 32);
    printf("String at address 0x%lx: %s\r\n", addr, data);
}