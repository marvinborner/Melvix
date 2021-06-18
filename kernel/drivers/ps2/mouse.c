// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <dev.h>
#include <drivers/cpu.h>
#include <drivers/int.h>
#include <drivers/ps2.h>
#include <errno.h>
#include <mem.h>
#include <print.h>
#include <proc.h>
#include <stack.h>
#include <str.h>
#include <sys.h>

PROTECTED static struct stack *queue = NULL;

PROTECTED static u8 wheel = 0;
PROTECTED static u8 extra_buttons = 0;

static char mouse_cycle = 0;
static char mouse_byte[4] = { 0 };
static void mouse_finish(void)
{
	struct event_mouse *event = zalloc(sizeof(*event));
	event->magic = MOUSE_MAGIC;
	event->pos = vec2(mouse_byte[1], mouse_byte[2]);
	event->rel = 1;
	event->scroll = mouse_byte[3] & 0x0f;
	event->scroll = event->scroll == 0x0f ? -1 : event->scroll; // Weird nibble stuff
	event->but.left = mouse_byte[0] & 1;
	event->but.right = (mouse_byte[0] >> 1) & 1;
	event->but.middle = (mouse_byte[0] >> 2) & 1;
	stack_push_bot(queue, event);
	mouse_cycle = 0;
	dev_unblock(DEV_MOUSE);
}

static void mouse_handler(void)
{
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
		if (wheel) {
			mouse_cycle++;
			break;
		}
		mouse_finish();
		break;
	case 3:
		mouse_byte[3] = ps2_read_data();
		mouse_finish();
		break;
	default:
		panic("Unknown mouse state\n");
		break;
	}
}

static res mouse_ready(void)
{
	return !stack_empty(queue) ? EOK : -EAGAIN;
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

CLEAR static u8 mouse_id(u8 device)
{
	ps2_write_device(device, 0xf2);
	return ps2_read_data();
}

CLEAR static void mouse_rate(u8 device, u8 rate)
{
	ps2_write_device(device, 0xf3);
	ps2_write_device(device, rate);
}

CLEAR void ps2_mouse_enable(u8 device)
{
	// Enable auxiliary mouse device
	ps2_write_device(device, 0xa8);

	// Use default settings
	ps2_write_device(device, 0xf6);

	// Enable mouse
	ps2_write_device(device, 0xf4);

	// Verify ID
	u8 id = mouse_id(device);
	assert(PS2_MOUSE(id));

	// Enable wheel
	if (id != PS2_TYPE_WHEEL_MOUSE) {
		mouse_rate(device, 200);
		mouse_rate(device, 100);
		mouse_rate(device, 80);
		id = mouse_id(device);
		if (id == PS2_TYPE_WHEEL_MOUSE)
			wheel = 1;
	}

	// Enable extra buttons
	if (id == PS2_TYPE_WHEEL_MOUSE) {
		mouse_rate(device, 200);
		mouse_rate(device, 200);
		mouse_rate(device, 80);
		id = mouse_id(device);
		if (id == PS2_TYPE_BUTTON_MOUSE)
			extra_buttons = 1;
	}

	mouse_rate(device, 20);
}

CLEAR void ps2_mouse_install(u8 device)
{
	ps2_mouse_enable(device);

	int_event_handler_add(12, mouse_handler);

	queue = stack_new();
	struct dev_dev *dev = zalloc(sizeof(*dev));
	dev->read = mouse_read;
	dev->ready = mouse_ready;
	dev_add(DEV_MOUSE, dev);
}
