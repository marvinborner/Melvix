#ifndef MELVIX_VGA_H
#define MELVIX_VGA_H

#include <stddef.h>
#include <stdint.h>
#include "vesa.h"

/**
 * Linked table of colors and hardware color codes
 */
enum vga_color;

/**
 * Initialize the terminal color and cursor position
 */
void terminal_initialize(void);

/**
 * Set the terminal color to a specified hardware color code
 * @see enum vga_color
 * @param color
 */
void terminal_set_color(uint8_t color);

/**
 * Clear the entire terminal screen
 */
void terminal_clear();

/**
 * Write a string to the terminal
 * @param data The string that should be written
 */
void terminal_write_string(const char *data);

/**
 * Put a new char at the x+1 cursor position and
 * handle according events (e.g. overflow, linebreak)
 * @param c The character (can also be \n or \r)
 */
void terminal_put_char(char c);

/**
 * Put a new char at the x+1 cursor position,
 * handle according events (e.g. overflow, linebreak) and
 * execute the current command if c is linebreak (\n)
 * @param c The character (can also be \n or \r)
 */
void terminal_put_keyboard_char(char c);

/**
 * Write a line to the terminal
 * @param data The line (string) that should be written
 */
void terminal_write_line(const char *data);

#endif