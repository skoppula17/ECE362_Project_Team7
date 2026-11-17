
#ifndef LCD_H
#define LCD_H

#include <stdint.h>

// LCD dimensions (portrait orientation)
#define LCD_WIDTH  240
#define LCD_HEIGHT 320

// RGB565 colors
#define LCD_BLACK   0x0000
#define LCD_WHITE   0xFFFF
#define LCD_RED     0xF800
#define LCD_GREEN   0x07E0
#define LCD_BLUE    0x001F
#define LCD_YELLOW  0xFFE0

// Initialize SPI and the TFT LCD
void lcd_init(void);

// Fill the entire screen with one color
void lcd_fill_screen(uint16_t color);

// Draw a filled rectangle (pixel coordinates)
void lcd_fill_rect(int x, int y, int w, int h,
                   uint16_t color);

// Draw one Snake grid cell (grid coordinates)
void lcd_draw_cell(int x, int y, uint16_t color);

#endif
