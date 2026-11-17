
#include "lcd.h"
#include "audio.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define LCD_SPI spi0

// TFT LCD pin assignments
#define PIN_SCK   18
#define PIN_MOSI  19
#define PIN_CS    20
#define PIN_RST   21
#define PIN_DC    22

// Existing Lab 6 display chip select
#define OLD_LCD_CS 17

// Size of each Snake grid cell
#define CELL_SIZE 10

// Send one command to the LCD
static void lcd_command(uint8_t cmd) {
    gpio_put(PIN_DC, 0);
    gpio_put(PIN_CS, 0);

    spi_write_blocking(LCD_SPI, &cmd, 1);

    gpio_put(PIN_CS, 1);
}

// Send data bytes to the LCD
static void lcd_data(const uint8_t *data, size_t len) {
    gpio_put(PIN_DC, 1);
    gpio_put(PIN_CS, 0);

    spi_write_blocking(LCD_SPI, data, len);

    gpio_put(PIN_CS, 1);
}

// Send a command followed by data
static void lcd_register(uint8_t cmd,
                         const uint8_t *data,
                         size_t len) {
    lcd_command(cmd);

    if (len > 0) {
        lcd_data(data, len);
    }
}

// Initialize SPI and LCD controller
void lcd_init(void) {
    // Deselect previous SPI display
    gpio_init(OLD_LCD_CS);
    gpio_set_dir(OLD_LCD_CS, GPIO_OUT);
    gpio_put(OLD_LCD_CS, 1);

    // Configure TFT control pins
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);
    gpio_put(PIN_RST, 1);

    gpio_init(PIN_DC);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_put(PIN_DC, 1);

    // Initialize SPI0 at 1 MHz
    spi_init(LCD_SPI, 1000 * 1000);

    spi_set_format(LCD_SPI, 8,
                   SPI_CPOL_0,
                   SPI_CPHA_0,
                   SPI_MSB_FIRST);

    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Hardware reset
    gpio_put(PIN_RST, 0);
    sleep_ms(20);
    gpio_put(PIN_RST, 1);
    sleep_ms(150);

    // ILI9341 initialization (provisional)
    lcd_command(0x01); // Software reset
    sleep_ms(150);

    lcd_command(0x11); // Exit sleep mode
    sleep_ms(120);

    // 16-bit RGB565 color
    uint8_t pixel_format = 0x55;
    lcd_register(0x3A, &pixel_format, 1);

    // Memory access control
    uint8_t madctl = 0x48;
    lcd_register(0x36, &madctl, 1);

    // Turn display on
    lcd_command(0x29);
    sleep_ms(20);
}

// Set the rectangular drawing area
static void lcd_window(int x0, int y0,
                       int x1, int y1) {
    uint8_t data[4];

    // Column addresses
    data[0] = x0 >> 8;
    data[1] = x0 & 0xFF;
    data[2] = x1 >> 8;
    data[3] = x1 & 0xFF;
    lcd_register(0x2A, data, 4);

    // Row addresses
    data[0] = y0 >> 8;
    data[1] = y0 & 0xFF;
    data[2] = y1 >> 8;
    data[3] = y1 & 0xFF;
    lcd_register(0x2B, data, 4);

    // Memory write
    lcd_command(0x2C);
}

// Draw a filled rectangle
void lcd_fill_rect(int x, int y, int w, int h,
                   uint16_t color) {
    if (w <= 0 || h <= 0) return;

    // Clip rectangle to display boundaries
    if (x < 0) {
        w += x;
        x = 0;
    }

    if (y < 0) {
        h += y;
        y = 0;
    }

    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;

    if (w > LCD_WIDTH - x) {
        w = LCD_WIDTH - x;
    }

    if (h > LCD_HEIGHT - y) {
        h = LCD_HEIGHT - y;
    }

    if (w <= 0 || h <= 0) return;

    lcd_window(x, y, x + w - 1, y + h - 1);

    // RGB565 pixel buffer
    uint8_t buffer[128];

    for (int i = 0; i < 64; i++) {
        buffer[2 * i] = color >> 8;
        buffer[2 * i + 1] = color & 0xFF;
    }

    int remaining = w * h;

    gpio_put(PIN_DC, 1);
    gpio_put(PIN_CS, 0);

    while (remaining > 0) {
        int count = remaining > 64 ? 64 : remaining;

        spi_write_blocking(LCD_SPI,
                           buffer,
                           count * 2);

        remaining -= count;
        // A full-screen clear takes over a second at 1 MHz. Service the
        // single-core audio sequencer between these roughly 1 ms chunks.
        audio_update();
    }

    gpio_put(PIN_CS, 1);
}

// Fill the entire screen
void lcd_fill_screen(uint16_t color) {
    lcd_fill_rect(0, 0,
                  LCD_WIDTH, LCD_HEIGHT,
                  color);
}

// Draw one Snake grid cell
void lcd_draw_cell(int x, int y, uint16_t color) {
    lcd_fill_rect(x * CELL_SIZE,
                  y * CELL_SIZE,
                  CELL_SIZE,
                  CELL_SIZE,
                  color);
}
