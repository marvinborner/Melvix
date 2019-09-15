#include "../../interrupts/interrupts.h"
#include "../../io/io.h"
#include "../../graphics/vga.h"

unsigned char keymap[128] = {
        0 /*E*/, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0 /*C*/, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0 /*LS*/,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0 /*RS*/, '*',
        0, // Alt key
        ' ', // Space bar
        0, // Caps lock
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // F keys
        0, // Num lock
        0, // Scroll lock
        0, // Home key
        0, // Up arrow
        0, // Page up
        '-',
        0, // Left arrow
        0,
        0, // Right arrow
        '+',
        0, // End key
        0, // Down arrow
        0, // Page down
        0, // Insert key
        0, // Delete key
        0, 0, 0,
        0, // F11
        0, // F12
        0, // Other keys
};

void keyboard_handler(struct regs *r) {
    unsigned char scan_code;

    scan_code = receive(0x60);

    if (scan_code & 0x80) {
        // Release
    } else {
        terminal_put_char(keymap[scan_code]);
    }
}

/* Installs the keyboard handler into IRQ1 */
void keyboard_install() {
    irq_install_handler(1, keyboard_handler);
}
