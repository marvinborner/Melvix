#include <kernel/interrupts/interrupts.h>
#include <kernel/io/io.h>
#include <kernel/graphics/vesa.h>
#include <kernel/input/input.h>
#include <kernel/lib/string.h>
#include <kernel/memory/alloc.h>

int shift_pressed;
int control_pressed;

char keymap[128] = {
        0 /*E*/, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        17 /*C*/, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 14 /*LS*/,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 14 /*RS*/, '*',
        0, // Alt key
        ' ', // Space bar
        15, // Caps lock
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

char shift_keymap[128] = {
        0 /*E*/, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
        '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
        17 /*C*/, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 14 /*LS*/,
        '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 14 /*RS*/, '*',
        0, // Alt key
        ' ', // Space bar
        15, // Caps lock
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

char *handle_shift(int scan_code)
{
    char *current_keymap = keymap;
    if (shift_pressed) current_keymap = shift_keymap;

    if ((scan_code & 0x80) == 0) { // PRESS
        // TODO: Fix caps lock deactivation when pressing shift while (locked) shifted
        if (current_keymap[scan_code] == 14 || (current_keymap[scan_code] == 15 && !shift_pressed)) {
            shift_pressed = 1;
        } else if (current_keymap[scan_code] == 15 && shift_pressed) {
            shift_pressed = 0;
        }
    } else { // RELEASE
        char key = current_keymap[scan_code];
        if (key == 14) shift_pressed = 0;
    }

    return current_keymap;
}

void keyboard_handler(struct regs *r)
{
    unsigned char scan_code;

    scan_code = inb(0x60);
    char *current_keymap = handle_shift(scan_code);

    if ((scan_code & 0x80) == 0) { // PRESS
        if (current_keymap[scan_code] == 17)
            control_pressed = 1;

        if (control_pressed && current_keymap[scan_code] == 'l') {
            vesa_clear();
            return;
        }

        keyboard_char_buffer = current_keymap[scan_code];
        keyboard_buffer[strlen(keyboard_buffer)] = keyboard_char_buffer;
    } else { // RELEASE
        if (current_keymap[scan_code] == (int) 0xffffffb5) // TODO: IDK WHY -107?!
            control_pressed = 0;
    }
}

void keyboard_acknowledge()
{
    while (inb(0x60) != 0xfa);
}

void keyboard_rate()
{
    outb(0x60, 0xF3);
    keyboard_acknowledge();
    outb(0x60, 0x0); // Rate{00000} Delay{00} 0
}

void keyboard_clear_buffer()
{
    kfree(keyboard_buffer);
    keyboard_buffer = (char *) kmalloc(4096); // 4KiB
}

// Installs the keyboard handler into IRQ1
void keyboard_install()
{
    keyboard_clear_buffer();
    keyboard_rate();
    irq_install_handler(1, keyboard_handler);
    shift_pressed = 0;
    info("Installed keyboard handler");
}
