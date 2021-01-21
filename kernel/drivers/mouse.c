// MIT License, Copyright (c) 2020 Marvin Borner

#include <boot.h>
#include <cpu.h>
#include <fs.h>
#include <interrupts.h>
#include <mem.h>
#include <print.h>
#include <stack.h>
#include <str.h>
#include <sys.h>

static char mouse_cycle = 0;
static char mouse_byte[3] = { 0 };
static struct stack *queue = NULL;
static u32 dev_id = 0;

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
		stack_push_bot(queue, event);

		mouse_cycle = 0;
		break;
	default:
		break;
	}
}

void mouse_serial_wait(u8 a_type)
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

void mouse_serial_write(u8 a_write)
{
	mouse_serial_wait(1);
	outb(0x64, 0xD4);
	mouse_serial_wait(1);
	outb(0x60, a_write);
}

u8 mouse_serial_read(void)
{
	mouse_serial_wait(0);
	return inb(0x60);
}

u32 mouse_ready(void)
{
	return !stack_empty(queue);
}

u32 mouse_read(void *buf, u32 offset, u32 count, struct device *dev)
{
	(void)dev;
	if (stack_empty(queue))
		return 0;

	struct event *e = stack_pop(queue);
	memcpy(buf, (u8 *)e + offset, count);
	return count;
}

void mouse_install(void)
{
	u8 status;

	// Enable auxiliary mouse device
	mouse_serial_wait(1);
	outb(0x64, 0xA8);

	// Enable interrupts
	mouse_serial_wait(1);
	outb(0x64, 0x20);
	mouse_serial_wait(0);
	status = (u8)(inb(0x60) | 3);
	mouse_serial_wait(1);
	outb(0x64, 0x60);
	mouse_serial_wait(1);
	outb(0x60, status);

	// Enable mousewheel
	mouse_serial_write(0xF2);
	mouse_serial_read();
	mouse_serial_read();
	mouse_serial_write(0xF3);
	mouse_serial_read();
	mouse_serial_write(200);
	mouse_serial_read();
	mouse_serial_write(0xF3);
	mouse_serial_read();
	mouse_serial_write(100);
	mouse_serial_read();
	mouse_serial_write(0xF3);
	mouse_serial_read();
	mouse_serial_write(80);
	mouse_serial_read();
	mouse_serial_write(0xF2);
	mouse_serial_read();
	status = (u8)mouse_serial_read();
	if (status == 3)
		printf("Scrollwheel support!\n");

	// Activate 4th and 5th mouse buttons
	mouse_serial_write(0xF2);
	mouse_serial_read();
	mouse_serial_read();
	mouse_serial_write(0xF3);
	mouse_serial_read();
	mouse_serial_write(200);
	mouse_serial_read();
	mouse_serial_write(0xF3);
	mouse_serial_read();
	mouse_serial_write(200);
	mouse_serial_read();
	mouse_serial_write(0xF3);
	mouse_serial_read();
	mouse_serial_write(80);
	mouse_serial_read();
	mouse_serial_write(0xF2);
	mouse_serial_read();
	status = (u8)mouse_serial_read();
	if (status == 4)
		printf("4th and 5th mouse button support!\n");

	/* TODO: Fix mouse laggyness
	mouse_serial_write(0xE8);
	mouse_serial_read();
	mouse_serial_write(0x03);
	mouse_serial_read();
	mouse_serial_write(0xF3);
	mouse_serial_read();
	mouse_serial_write(200);
	mouse_serial_read(); */

	// Enable mouse
	mouse_serial_write(0xF4);
	mouse_serial_read();

	// Setup the mouse handler
	irq_install_handler(12, mouse_handler);

	queue = stack_new();
	struct device *dev = malloc(sizeof(*dev));
	dev->name = strdup("mouse");
	dev->type = DEV_CHAR;
	dev->read = mouse_read;
	dev->ready = mouse_ready;
	device_add(dev);
	dev_id = dev->id;
}
