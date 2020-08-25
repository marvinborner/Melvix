// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <def.h>
#include <event.h>
#include <interrupts.h>
#include <mem.h>
#include <print.h>
#include <sys.h>

char keymap[128];
struct event_keyboard *event;

int state = 0;
int merged = 0;
void keyboard_handler()
{
	u32 scancode = (u32)inb(0x60);

	// TODO: Support more than two-byte scancodes
	if (scancode == 0xe0) {
		merged = 0xe0;
		state = 1;
		return;
	} else {
		merged = scancode << 8 | merged;
	}

	// TODO: "Merge" scancode to linux keycode?
	/* printf("%x %x = %x\n", scancode, state ? 0xe0 : 0, merged); */

	free(event);
	event = malloc(sizeof(*event));
	event->magic = KEYBOARD_MAGIC;
	event->press = (scancode & 0x80) == 0;
	event->scancode = event->press ? scancode : scancode & ~0x80;
	event_trigger(EVENT_KEYBOARD, event);

	state = 0;
	merged = 0;
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
