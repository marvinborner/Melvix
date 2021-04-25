// MIT License, Copyright (c) 2021 Marvin Borner
// VMWare extensions/backdoors for better VM integration

#include <def.h>
#include <interrupts.h>
#include <io.h>
#include <mem.h>
#include <print.h>
#include <ps2.h>
#include <stack.h>
#include <vmware.h>

#define VMWARE_CMD_VERSION 0x0a

#define VMWARE_MAGIC 0x564d5868
#define VMWARE_PORT 0x5658

struct vmware_command {
	union {
		u32 ax;
		u32 magic;
	} a;
	union {
		u32 bx;
		u16 size;
	} b;
	union {
		u32 cx;
		u16 command;
	} c;
	union {
		u32 dx;
		u16 port;
	} d;
	u32 si;
	u32 di;
};

static void vmware_out(struct vmware_command *command)
{
	command->a.magic = VMWARE_MAGIC;
	command->d.port = VMWARE_PORT;
	command->si = 0;
	command->di = 0;
	__asm__ volatile("in %%dx, %0"
			 : "+a"(command->a.ax), "+b"(command->b.bx), "+c"(command->c.cx),
			   "+d"(command->d.dx), "+S"(command->si), "+D"(command->di));
}

CLEAR u8 vmware_detect(void)
{
	struct vmware_command command = { .b.bx = ~VMWARE_MAGIC, .c.command = VMWARE_CMD_VERSION };
	vmware_out(&command);
	return command.b.bx == VMWARE_MAGIC && command.a.ax != U32_MAX;
}

/**
 * VMWare mouse
 */

#define VMMOUSE_DATA 39
#define VMMOUSE_STATUS 40
#define VMMOUSE_COMMAND 41

#define VMMOUSE_QEMU 0x3442554a
#define VMMOUSE_READ 0x45414552
#define VMMOUSE_RELATIVE 0x4c455252
#define VMMOUSE_ABSOLUTE 0x53424152
#define VMMOUSE_LEFT_CLICK 0x20
#define VMMOUSE_RIGHT_CLICK 0x10
#define VMMOUSE_MIDDLE_CLICK 0x08

PROTECTED static struct stack *queue = NULL;

CLEAR u8 vmware_mouse_detect(void)
{
	struct vmware_command command = { .b.bx = VMMOUSE_READ, .c.command = VMMOUSE_COMMAND };
	vmware_out(&command);
	command.b.bx = 1;
	command.c.command = VMMOUSE_DATA;
	vmware_out(&command);
	return command.a.ax == VMMOUSE_QEMU;
}

CLEAR static void vmware_mouse_enable(void)
{
	struct vmware_command command = { .b.bx = 0, .c.command = VMMOUSE_STATUS };
	vmware_out(&command);
	if (command.a.ax == (0xffffu << 16))
		return;

	command.b.bx = VMMOUSE_ABSOLUTE;
	command.c.command = VMMOUSE_COMMAND;
	vmware_out(&command);
}

static void vmware_mouse_handler(struct regs *r)
{
	UNUSED(r);
	ps2_read_data(); // Unused, for PS/2 compatibility

	struct vmware_command command = { .b.bx = 0, .c.command = VMMOUSE_STATUS };
	vmware_out(&command);
	if (command.a.ax == (0xffffu << 16))
		return;

	u32 cnt = command.a.ax & 0xffff;
	if (!cnt || cnt % 4)
		return;

	command.b.size = 4;
	command.c.command = VMMOUSE_DATA;
	vmware_out(&command);

	u32 buttons = command.a.ax & 0xffff;
	s8 scroll = command.d.dx & 0xff;
	s32 x = command.b.bx;
	s32 y = command.c.cx;

	struct event_mouse *event = zalloc(sizeof(*event));
	event->magic = MOUSE_MAGIC;
	event->pos = vec2(x, y);
	event->rel = 0;
	event->scroll = scroll;
	event->but.left = (buttons & VMMOUSE_LEFT_CLICK) != 0;
	event->but.right = (buttons & VMMOUSE_RIGHT_CLICK) != 0;
	event->but.middle = (buttons & VMMOUSE_MIDDLE_CLICK) != 0;
	stack_push_bot(queue, event);
	io_unblock(IO_MOUSE);
}

static res vmware_mouse_ready(void)
{
	return !stack_empty(queue) ? EOK : -EAGAIN;
}

static res vmware_mouse_read(void *buf, u32 offset, u32 count)
{
	if (stack_empty(queue))
		return -EINVAL;

	struct event_mouse *e = stack_pop(queue);
	memcpy_user(buf, (u8 *)e + offset, MIN(count, sizeof(*e)));
	free(e);
	return MIN(count, sizeof(*e));
}

CLEAR void vmware_mouse_install(u8 device)
{
	// Enable auxiliary mouse device
	ps2_write_device(device, 0xa8);

	// Use default settings
	ps2_write_device(device, 0xf6);

	// Enable mouse
	ps2_write_device(device, 0xf4);

	vmware_mouse_enable();
	irq_install_handler(12, vmware_mouse_handler);

	queue = stack_new();
	struct io_dev *dev = zalloc(sizeof(*dev));
	dev->read = vmware_mouse_read;
	dev->ready = vmware_mouse_ready;
	io_add(IO_MOUSE, dev);
}
