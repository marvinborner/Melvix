#include <common.h>
#include <events/event.h>
#include <graphics/vesa.h>
#include <input/input.h>
#include <interrupts/interrupts.h>
#include <io/io.h>
#include <lib/string.h>
#include <memory/alloc.h>

u8 scancode;

void keyboard_handler(struct regs *r)
{
	scancode = inb(0x60);
	struct keyboard_event *event = malloc(sizeof(struct keyboard_event));
	event->scancode = scancode;
	event_trigger(MAP_KEYBOARD, (u8 *)event);
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

char wait_scancode()
{
	scancode = 0;
	while (scancode == 0) {
	};
	return scancode;
}

void keyboard_install()
{
	keyboard_rate();
	irq_install_handler(1, keyboard_handler);
	info("Installed keyboard handler");
}