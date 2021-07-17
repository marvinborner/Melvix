// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef VGA_H
#define VGA_H

#include <kernel.h>

void vga_clear(void);
void vga_put_at(char ch, u8 x, u8 y, u8 color);
void vga_print(const char *data);

#endif
