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

char keyboard_char_buffer;

#endif