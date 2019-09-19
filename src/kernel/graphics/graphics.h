#ifndef MELVIX_VGA_H
#define MELVIX_VGA_H

#include <stddef.h>
#include <stdint.h>

enum vga_color;

void terminal_initialize(void);

void terminal_set_color(uint8_t color);

void terminal_clear();

void terminal_write_string(const char *data);

void terminal_put_char(char c);

void terminal_write_line(const char *data);

// VESA
void vesa_init();

#endif