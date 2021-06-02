// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <drivers/cpu.h>
#include <drivers/int.h>
#include <drivers/ps2.h>
#include <errno.h>
#include <io.h>
#include <mem.h>
#include <print.h>
#include <proc.h>
#include <stack.h>
#include <str.h>
#include <sys.h>

PROTECTED static struct stack *queue = NULL;

static struct event_keyboard *event = NULL;
static int state = 0;
static int merged = 0;
static void keyboard_handler(void)
{
	u8 scancode = ps2_read_data();

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
	stack_push_bot(queue, event);

	state = 0;
	merged = 0;

	io_unblock(IO_KEYBOARD);
}

static res keyboard_read(void *buf, u32 offset, u32 count)
{
	if (stack_empty(queue))
		return -EINVAL;

	struct event_keyboard *e = stack_pop(queue);
	memcpy_user(buf, (u8 *)e + offset, MIN(count, sizeof(*e)));
	free(e);
	return MIN(count, sizeof(*e));
}

static res keyboard_ready(void)
{
	return !stack_empty(queue) ? EOK : -EAGAIN;
}

CLEAR void ps2_keyboard_reset(void)
{
	if (queue)
		stack_clear(queue);
}

CLEAR void ps2_keyboard_install(u8 device)
{
	UNUSED(device);

	int_event_handler_add(1, keyboard_handler);

	queue = stack_new();
	struct io_dev *dev = zalloc(sizeof(*dev));
	dev->read = keyboard_read;
	dev->ready = keyboard_ready;
	io_add(IO_KEYBOARD, dev);
}
