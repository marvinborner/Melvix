// MIT License, Copyright (c) 2020 Marvin Borner

#include <boot.h>
#include <cpu.h>
#include <event.h>
#include <interrupts.h>
#include <mem.h>
#include <print.h>
#include <sys.h>

static char mouse_cycle = 0;
static char mouse_byte[3] = { 0 };

static struct event_mouse *event = NULL;

void mouse_handler()
{
	switch (mouse_cycle) {
	case 0:
		mouse_byte[0] = (char)inb(0x60);
		if (((mouse_byte[0] >> 3) & 1) == 1)
			mouse_cycle++;
		else
			mouse_cycle = 0;
		break;
	case 1:
		mouse_byte[1] = (char)inb(0x60);
		mouse_cycle++;
		break;
	case 2:
		mouse_byte[2] = (char)inb(0x60);

		if (event)
			free(event);
		event = malloc(sizeof(*event));
		event->magic = MOUSE_MAGIC;
		event->diff_x = mouse_byte[1];
		event->diff_y = mouse_byte[2];
		event->but1 = mouse_byte[0] & 1;
		event->but2 = (mouse_byte[0] >> 1) & 1;
		event->but3 = (mouse_byte[0] >> 2) & 1;
		event_trigger(EVENT_MOUSE, event);

		mouse_cycle = 0;
		break;
	default:
		break;
	}
}

void mouse_wait(u8 a_type)
{
	u32 time_out = 100000;
	if (a_type == 0) {
		while (time_out--)
			if ((inb(0x64) & 1) == 1)
				return;
		return;
	} else {
		while (time_out--)
			if ((inb(0x64) & 2) == 0)
				return;
		return;
	}
}

void mouse_write(u8 a_write)
{
	mouse_wait(1);
	outb(0x64, 0xD4);
	mouse_wait(1);
	outb(0x60, a_write);
}

u8 mouse_read(void)
{
	mouse_wait(0);
	return inb(0x60);
}

void mouse_install(void)
{
	u8 status;

	// Enable auxiliary mouse device
	mouse_wait(1);
	outb(0x64, 0xA8);

	// Enable interrupts
	mouse_wait(1);
	outb(0x64, 0x20);
	mouse_wait(0);
	status = (u8)(inb(0x60) | 3);
	mouse_wait(1);
	outb(0x64, 0x60);
	mouse_wait(1);
	outb(0x60, status);

	// Enable mousewheel
	mouse_write(0xF2);
	mouse_read();
	mouse_read();
	mouse_write(0xF3);
	mouse_read();
	mouse_write(200);
	mouse_read();
	mouse_write(0xF3);
	mouse_read();
	mouse_write(100);
	mouse_read();
	mouse_write(0xF3);
	mouse_read();
	mouse_write(80);
	mouse_read();
	mouse_write(0xF2);
	mouse_read();
	status = (u8)mouse_read();
	if (status == 3)
		printf("Scrollwheel support!\n");

	// Activate 4th and 5th mouse buttons
	mouse_write(0xF2);
	mouse_read();
	mouse_read();
	mouse_write(0xF3);
	mouse_read();
	mouse_write(200);
	mouse_read();
	mouse_write(0xF3);
	mouse_read();
	mouse_write(200);
	mouse_read();
	mouse_write(0xF3);
	mouse_read();
	mouse_write(80);
	mouse_read();
	mouse_write(0xF2);
	mouse_read();
	status = (u8)mouse_read();
	if (status == 4)
		printf("4th and 5th mouse button support!\n");

	/* TODO: Fix mouse laggyness
	mouse_write(0xE8);
	mouse_read();
	mouse_write(0x03);
	mouse_read();
	mouse_write(0xF3);
	mouse_read();
	mouse_write(200);
	mouse_read(); */

	// Enable mouse
	mouse_write(0xF4);
	mouse_read();

	// Setup the mouse handler
	irq_install_handler(12, mouse_handler);
}
