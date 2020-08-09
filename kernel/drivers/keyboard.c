#include <cpu.h>
#include <def.h>
#include <gui.h>
#include <interrupts.h>

char keymap[128];

// TODO: Use keyboard as event and move logic to other file
void keyboard_handler()
{
	u8 scan_code = inb(0x60);

	if (scan_code > 128)
		return;

	if ((scan_code & 0x80) == 0) { // PRESS
		gui_term_write_char(keymap[scan_code]);
	}
}

void keyboard_acknowledge()
{
	while (inb(0x60) != 0xfa)
		;
}

void keyboard_rate()
{
	outb(0x60, 0xF3);
	keyboard_acknowledge();
	outb(0x60, 0x0); // Rate{00000} Delay{00} 0
}

void keyboard_install()
{
	//keyboard_rate(); TODO: Fix keyboard rate?
	irq_install_handler(1, keyboard_handler);
}

char keymap[128] = {
	0,    27,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-',  '=',
	'\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',  ']',
	'\n', 17,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	14,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 14,   '*',
	0, // Alt key
	' ', // Space bar
	15, // Caps lock
	0,    0,    0,	 0,   0,   0,	0,   0,	  0,   0, // F keys
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
	0,    0,    0,
	0, // F11
	0, // F12
	0, // Other keys
};
