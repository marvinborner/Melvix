#include <stddef.h>
#include <stdint.h>
#include "../io/io.h"

// Hardware text mode color constants
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t *terminal_buffer;

void terminal_clear() {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    send(0x3D4, 0x0A);
    send(0x3D5, (receive(0x3D5) & 0xC0) | cursor_start);
    send(0x3D4, 0x0B);
    send(0x3D5, (receive(0x3D5) & 0xE0) | cursor_end);
}

void terminal_update_cursor(void) {
    unsigned temp = terminal_row * VGA_WIDTH + terminal_column;
    send(0x3D4, 14);
    send(0x3D5, temp >> 8);
    send(0x3D4, 15);
    send(0x3D5, temp);
}

void terminal_initialize(void) {
    terminal_enable_cursor(0, 15);
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t *) 0xB8000;
    terminal_clear();
}

void terminal_scroll(void) {
    if (terminal_row >= VGA_HEIGHT) {
        terminal_row = VGA_HEIGHT - 1;
        for (size_t x = 0; x < VGA_WIDTH; x++)
            for (size_t y = 0; y < VGA_HEIGHT; y++) {
                uint16_t c = terminal_buffer[y * VGA_WIDTH + x];
                terminal_buffer[(y - 1) * VGA_WIDTH + x] = c;
                terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
            }
    }
}

void terminal_set_color(uint8_t color) {
    terminal_color = color;
}

void terminal_put_entry_at(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_put_char(char c) {
    if (c == 0x08) {
        if (terminal_column != 0) terminal_column--;
    } else if (c == 0x09) {
        terminal_column = (terminal_column + 8) & ~(8 - 1);
    } else if (c == '\r') {
        terminal_column = 0;
    } else if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        terminal_scroll();
        terminal_put_entry_at('$', terminal_color, terminal_column, terminal_row);
        terminal_column = 2;
    } else if (c >= ' ') { // Any printable character
        terminal_put_entry_at(c, terminal_color, terminal_column, terminal_row);
        terminal_column++;
    }

    // Add new line on overflow
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }

    terminal_scroll();
    terminal_update_cursor();
}

void terminal_write(const char *data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_put_char(data[i]);
}

void terminal_write_string(const char *data) {
    terminal_write(data, strlen(data));
}
