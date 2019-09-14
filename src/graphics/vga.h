#ifndef MELVIX_VGA_H
#define MELVIX_VGA_H

#include <stddef.h>
#include <stdint.h>

enum vga_color;

void terminal_initialize(void);

void terminal_set_color(uint8_t color);

void terminal_write_string(const char *data);

#endif