// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <def.h>
#include <event.h>
#include <interrupts.h>
#include <mem.h>
#include <print.h>
#include <sys.h>

static struct event_keyboard *event = NULL;

static int state = 0;
static int merged = 0;
void keyboard_handler()
{
	int scancode = inb(0x60);

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

	event = malloc(sizeof(*event));
	event->magic = KEYBOARD_MAGIC;
	event->press = (scancode & 0x80) == 0;
	event->scancode = event->press ? scancode : scancode & ~0x80;
	event_trigger(EVENT_KEYBOARD, event);

	state = 0;
	merged = 0;
}

void keyboard_acknowledge(void)
{
	while (inb(0x60) != 0xfa)
		;
}

void keyboard_rate(void)
{
	outb(0x60, 0xF3);
	keyboard_acknowledge();
	outb(0x60, 0x0); // Rate{00000} Delay{00} 0
}

void keyboard_install(void)
{
	//keyboard_rate(); TODO: Fix keyboard rate?
	irq_install_handler(1, keyboard_handler);
}
