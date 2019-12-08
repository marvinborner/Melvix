#ifndef MELVIX_INPUT_H
#define MELVIX_INPUT_H

/**
 * Initialize the mouse IRQ handler
 */
void mouse_install();

/**
 * Initialize the us keyboard layout,
 * keyboard rate and IRQ handler
 */
void keyboard_install();

void keyboard_clear_buffer();

char keyboard_char_buffer;

char *keyboard_buffer;

#endif
