#ifndef MELVIX_INPUT_H
#define MELVIX_INPUT_H

#include <stdint.h>

/**
 * Initialize the mouse IRQ handler
 */
void mouse_install();

/**
 * Initialize the us keyboard layout,
 * keyboard rate and IRQ handler
 */
void keyboard_install();

char wait_scancode();

#endif