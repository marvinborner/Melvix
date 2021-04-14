// MIT License, Copyright (c) 2020 Marvin Borner

#include <boot.h>
#include <cpu.h>
#include <errno.h>
#include <fs.h>
#include <interrupts.h>
#include <mem.h>
#include <print.h>
#include <proc.h>
#include <ps2.h>
#include <stack.h>
#include <str.h>
#include <sys.h>

PROTECTED static struct stack *queue = NULL;
PROTECTED static u32 dev_id = 0;

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

#define MOUSE_WAIT_OUT 0
#define MOUSE_WAIT_IN 1
CLEAR static void mouse_serial_wait(u8 in)
{
	u32 time_out = 100000;
	if (in) {
		while (time_out--)
			if (ps2_read_status().in_full)
				return;
		return;
	} else {
		while (time_out--)
			if (ps2_read_status().out_full)
				return;
		return;
	}
}

CLEAR static void mouse_serial_write(u8 data)
{
	mouse_serial_wait(MOUSE_WAIT_IN);
	ps2_write_command(0xd4);
	mouse_serial_wait(MOUSE_WAIT_IN);
	ps2_write_data(data);
}

CLEAR static u8 mouse_serial_read(void)
{
	mouse_serial_wait(MOUSE_WAIT_OUT);
	return ps2_read_data();
}

static res mouse_ready(void)
{
	return !stack_empty(queue);
}

static res mouse_read(void *buf, u32 offset, u32 count, struct vfs_dev *dev)
{
	(void)dev;
	if (stack_empty(queue))
		return -EINVAL;

	struct event_mouse *e = stack_pop(queue);
	memcpy_user(buf, (u8 *)e + offset, MIN(count, sizeof(*e)));
	free(e);
	return MIN(count, sizeof(*e));
}

CLEAR void ps2_mouse_install(void)
{
	u8 status;

	// Enable auxiliary mouse device
	mouse_serial_wait(MOUSE_WAIT_IN);
	ps2_write_command(0xa8);

	// Enable interrupts
	mouse_serial_wait(MOUSE_WAIT_IN);
	ps2_write_command(0x20);
	mouse_serial_wait(MOUSE_WAIT_OUT);
	status = ps2_read_data() | 3;
	mouse_serial_wait(MOUSE_WAIT_IN);
	ps2_write_command(0x60);
	mouse_serial_wait(MOUSE_WAIT_IN);
	ps2_write_data(status);

	// Use default settings
	mouse_serial_write(0xf6);
	mouse_serial_read();

	// Enable mousewheel
	mouse_serial_write(0xf2);
	mouse_serial_read();
	mouse_serial_read();
	mouse_serial_write(0xf3);
	mouse_serial_read();
	mouse_serial_write(200);
	mouse_serial_read();
	mouse_serial_write(0xf3);
	mouse_serial_read();
	mouse_serial_write(100);
	mouse_serial_read();
	mouse_serial_write(0xf3);
	mouse_serial_read();
	mouse_serial_write(80);
	mouse_serial_read();
	mouse_serial_write(0xf2);
	mouse_serial_read();
	status = (u8)mouse_serial_read();
	if (status == 3) {
	}
	/* printf("Scrollwheel support!\n"); */

	// Activate 4th and 5th mouse buttons
	mouse_serial_write(0xf2);
	mouse_serial_read();
	mouse_serial_read();
	mouse_serial_write(0xf3);
	mouse_serial_read();
	mouse_serial_write(200);
	mouse_serial_read();
	mouse_serial_write(0xf3);
	mouse_serial_read();
	mouse_serial_write(200);
	mouse_serial_read();
	mouse_serial_write(0xf3);
	mouse_serial_read();
	mouse_serial_write(80);
	mouse_serial_read();
	mouse_serial_write(0xf2);
	mouse_serial_read();
	status = (u8)mouse_serial_read();
	if (status == 4) {
	}
	/* printf("4th and 5th mouse button support!\n"); */

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
	mouse_serial_write(0xf4);
	mouse_serial_read();

	// Setup the mouse handler
	irq_install_handler(12, mouse_handler);

	queue = stack_new();
	struct vfs_dev *dev = zalloc(sizeof(*dev));
	dev->name = strdup("mouse");
	dev->type = DEV_CHAR;
	dev->read = mouse_read;
	/* device_add(dev); */
	dev_id = dev->id;
}
