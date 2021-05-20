// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <bus.h>
#include <def.h>
#include <drivers/cpu.h>
#include <drivers/interrupts.h>
#include <drivers/ps2.h>
#include <drivers/timer.h>
#include <drivers/vbe.h>
#include <drivers/vmware.h>
#include <io.h>
#include <list.h>
#include <logger.h>
#include <mem.h>
#include <mm.h>
#include <multiboot.h>
#include <proc.h>
#include <rand.h>
#include <str.h>
#include <syscall.h>

struct io_listener {
	u32 group;
	struct proc *proc;
};

PROTECTED static struct io_dev *io_mappings[IO_MAX] = { 0 };
PROTECTED static struct list *io_listeners[IO_MAX] = { 0 };

static u32 group_id = 0;

static u8 io_type_valid(enum io_type io)
{
	return io > IO_MIN && io < IO_MAX;
}

static struct io_dev *io_get(enum io_type io)
{
	if (!io_type_valid(io))
		return NULL;

	return io_mappings[io];
}

// TODO: Efficiency
static void io_remove_group(u32 group)
{
	for (u32 io = IO_MIN; io < IO_MAX; io++) {
		struct node *iterator = io_listeners[io]->head;
		while (iterator) {
			struct io_listener *listener = iterator->data;
			struct node *next = iterator->next;
			if (listener->group == group)
				list_remove(io_listeners[io], iterator);
			iterator = next;
		}
	}

	if (group + 1 == group_id)
		group_id--;
}

CLEAR void io_add(enum io_type io, struct io_dev *dev)
{
	assert(io_type_valid(io) && !io_mappings[io]);
	io_mappings[io] = dev;
}

res io_poll(u32 *devs)
{
	if (!memory_readable(devs))
		return -EFAULT;

	u32 group = group_id++;

	for (u32 *p = devs; p && memory_readable(p); p++) {
		stac();
		enum io_type io = *p;
		clac();

		if (!io)
			break;

		struct io_dev *dev = io_get(io);
		if (!dev || !dev->read) {
			io_remove_group(group);
			return -ENOENT;
		}

		if (dev->ready) {
			res ready = dev->ready();
			if (ready == EOK) {
				io_remove_group(group);
				return io;
			} else if (ready != -EAGAIN) {
				return ready;
			}
		}

		struct io_listener *listener = zalloc(sizeof(*listener));
		listener->group = group;
		listener->proc = proc_current();
		list_add(io_listeners[io], listener);
	}

	proc_state(proc_current(), PROC_BLOCKED);
	proc_yield();
	return io_poll(devs);
}

res io_control(enum io_type io, u32 request, void *arg1, void *arg2, void *arg3)
{
	struct io_dev *dev;
	if (!(dev = io_get(io)) || !dev->control)
		return -ENOENT;

	return dev->control(request, arg1, arg2, arg3);
}

res io_write(enum io_type io, const void *buf, u32 offset, u32 count)
{
	if (!memory_readable_range(memory_range(buf, count)))
		return -EFAULT;

	struct io_dev *dev;
	if (!(dev = io_get(io)) || !dev->write)
		return -ENOENT;

	return dev->write(buf, offset, count);
}

res io_read(enum io_type io, void *buf, u32 offset, u32 count)
{
	if (!memory_writable_range(memory_range(buf, count)))
		return -EFAULT;

	struct io_dev *dev;
	if (!(dev = io_get(io)) || !dev->read)
		return -ENOENT;

	if (dev->ready && dev->ready() != EOK)
		return -EAGAIN;

	return dev->read(buf, offset, count);
}

res io_ready(enum io_type io)
{
	struct io_dev *dev;
	if (!(dev = io_get(io)))
		return -ENOENT;

	if (dev->ready && dev->ready() != EOK)
		return -EAGAIN;

	return EOK;
}

void io_block(enum io_type io, struct proc *proc)
{
	assert(io_type_valid(io));
	struct io_listener *listener = zalloc(sizeof(*listener));
	listener->group = group_id++;
	listener->proc = proc;
	list_add(io_listeners[io], listener);
	proc_state(proc_current(), PROC_BLOCKED);
	proc_yield();
}

void io_unblock(enum io_type io)
{
	assert(io_type_valid(io));
	struct node *iterator = io_listeners[io]->head;
	while (iterator) {
		struct io_listener *listener = iterator->data;
		struct proc *proc = listener->proc;
		proc_state(proc, PROC_RUNNING);
		struct node *next = iterator->next;
		io_remove_group(listener->group);
		free(listener);
		iterator = next;
	}

	if (proc_idle())
		proc_yield();
}

void io_unblock_pid(u32 pid)
{
	for (u32 io = IO_MIN; io < IO_MAX; io++) {
		struct node *iterator = io_listeners[io]->head;
		while (iterator) {
			struct io_listener *listener = iterator->data;
			struct proc *proc = listener->proc;
			proc_state(proc, PROC_RUNNING);
			struct node *next = iterator->next;
			if (proc->pid == pid) {
				list_remove(io_listeners[io], iterator);
				free(listener);
			}
			iterator = next;
		}
	}

	if (proc_idle())
		proc_yield();
}

CLEAR void io_install(void)
{
	for (u32 i = 0; i < IO_MAX; i++)
		io_listeners[i] = list_new();

	/**
	 * Keyboard & mouse detection
	 */

	ps2_detect();

	u8 ps2_keyboard = ps2_keyboard_detect();
	if (ps2_keyboard != U8_MAX) {
		ps2_keyboard_install(ps2_keyboard);
	}

	u8 ps2_mouse = ps2_mouse_detect();
	if (ps2_mouse != U8_MAX) {
		if (vmware_detect() && vmware_mouse_detect())
			vmware_mouse_install(ps2_mouse);
		else
			ps2_mouse_install(ps2_mouse);
	}

	/**
	 * Framebuffer detection
	 */

	u32 vbe = multiboot_vbe();
	if (vbe)
		vbe_install(vbe);

	/**
	 * Other devices
	 */

	timer_install();
	logger_install();
	bus_install();
}
