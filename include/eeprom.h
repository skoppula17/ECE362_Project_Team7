#ifndef EEPROM_H
#define EEPROM_H

#include "pico/stdlib.h"
void init_i2c();
void eeprom_write(uint16_t loc, const char* data, uint8_t len);
void eeprom_read(uint16_t loc, char data[], uint8_t len);

#endif