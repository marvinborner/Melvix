#include <cpu.h>
#include <def.h>
#include <interrupts.h>
#include <serial.h>
#include <vesa.h>

u8 scancode;

void keyboard_handler()
{
	scancode = inb(0x60);
	//serial_print("KEY\n");
	//struct keyboard_event *event = malloc(sizeof(struct keyboard_event));
	//event->scancode = scancode;
	//event_trigger(MAP_KEYBOARD, (u8 *)event);
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
	//keyboard_rate(); TODO: Fix keyboard rate?
	irq_install_handler(1, keyboard_handler);
}
