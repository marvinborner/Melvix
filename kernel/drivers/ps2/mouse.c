// MIT License, Copyright (c) 2020 Marvin Borner

#include <boot.h>
#include <cpu.h>
#include <errno.h>
#include <interrupts.h>
#include <io.h>
#include <mem.h>
#include <print.h>
#include <proc.h>
#include <ps2.h>
#include <stack.h>
#include <str.h>
#include <sys.h>

PROTECTED static struct stack *queue = NULL;

static struct event_mouse *event = NULL;
static char mouse_cycle = 0;
static char mouse_byte[3] = { 0 };
static void mouse_handler(struct regs *r)
{
	UNUSED(r);
	switch (mouse_cycle) {
	case 0:
		mouse_byte[0] = ps2_read_data();
		if (((mouse_byte[0] >> 3) & 1) == 1)
			mouse_cycle++;
		else
			mouse_cycle = 0;
		break;
	case 1:
		mouse_byte[1] = ps2_read_data();
		mouse_cycle++;
		break;
	case 2:
		mouse_byte[2] = ps2_read_data();

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

static res mouse_ready(void)
{
	return !stack_empty(queue);
}

static res mouse_read(void *buf, u32 offset, u32 count)
{
	if (stack_empty(queue))
		return -EINVAL;

	struct event_mouse *e = stack_pop(queue);
	memcpy_user(buf, (u8 *)e + offset, MIN(count, sizeof(*e)));
	free(e);
	return MIN(count, sizeof(*e));
}

CLEAR void ps2_mouse_install(u8 device)
{
	u8 status;

	// Enable auxiliary mouse device
	ps2_write_device(device, 0xa8);

	// Enable interrupts
	ps2_write_command(0x20);
	status = ps2_read_data() | 3;
	ps2_write_command(0x60);
	ps2_write_data(status);

	// Use default settings
	ps2_write_device(device, 0xf6);
	ps2_read_data();

	// Enable mouse
	ps2_write_device(device, 0xf4);
	ps2_read_data();

	// Setup the mouse handler
	irq_install_handler(12, mouse_handler);

	queue = stack_new();
	struct io_dev *dev = zalloc(sizeof(*dev));
	dev->read = mouse_read;
	dev->ready = mouse_ready;
	io_add(IO_MOUSE, dev);
}
