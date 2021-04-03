// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <def.h>
#include <errno.h>
#include <fs.h>
#include <interrupts.h>
#include <keyboard.h>
#include <mem.h>
#include <print.h>
#include <proc.h>
#include <stack.h>
#include <str.h>
#include <sys.h>

PROTECTED static struct stack *queue = NULL;
PROTECTED static u32 dev_id = 0;

static struct event_keyboard *event = NULL;
static int state = 0;
static int merged = 0;
static void keyboard_handler(struct regs *r)
{
	UNUSED(r);
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
	stack_push_bot(queue, event);

	state = 0;
	merged = 0;

	proc_unblock(dev_id, PROC_BLOCK_DEV);
}

/*static void keyboard_acknowledge(void)
{
	while (inb(0x60) != 0xfa)
		;
}

static void keyboard_rate(void)
{
	outb(0x60, 0xF3);
	keyboard_acknowledge();
	outb(0x60, 0x0); // Rate{00000} Delay{00} 0
}*/

static res keyboard_read(void *buf, u32 offset, u32 count, struct device *dev)
{
	UNUSED(dev);
	if (stack_empty(queue))
		return -EINVAL;

	struct event_keyboard *e = stack_pop(queue);
	memcpy_user(buf, (u8 *)e + offset, MIN(count, sizeof(*e)));
	free(e);
	return MIN(count, sizeof(*e));
}

static res keyboard_ready(void)
{
	return !stack_empty(queue);
}

CLEAR void keyboard_reset(void)
{
	stack_clear(queue);
}

CLEAR void keyboard_install(void)
{
	//keyboard_rate(); TODO: Fix keyboard rate?
	irq_install_handler(1, keyboard_handler);

	queue = stack_new();
	struct device *dev = zalloc(sizeof(*dev));
	dev->name = strdup("kbd");
	dev->type = DEV_CHAR;
	dev->read = keyboard_read;
	dev->ready = keyboard_ready;
	device_add(dev);
	dev_id = dev->id;
}
