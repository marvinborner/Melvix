// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <bus.h>
#include <def.h>
#include <dev.h>
#include <drivers/bga.h>
#include <drivers/cpu.h>
#include <drivers/int.h>
#include <drivers/ps2.h>
#include <drivers/vbe.h>
#include <drivers/vmware.h>
#include <fb.h>
#include <list.h>
#include <logger.h>
#include <mem.h>
#include <mm.h>
#include <multiboot.h>
#include <proc.h>
#include <rand.h>
#include <str.h>
#include <syscall.h>
#include <timer.h>

struct dev_listener {
	u32 group;
	struct proc *proc;
};

PROTECTED static struct dev_dev *dev_mappings[DEV_MAX] = { 0 };
PROTECTED static struct list *dev_listeners[DEV_MAX] = { 0 };

static u32 group_id = 0;

static u8 dev_type_valid(enum dev_type type)
{
	return type > DEV_MIN && type < DEV_MAX;
}

static struct dev_dev *dev_get(enum dev_type type)
{
	if (!dev_type_valid(type))
		return NULL;

	return dev_mappings[type];
}

// TODO: Efficiency
static void dev_remove_group(u32 group)
{
	for (u32 dev = DEV_MIN; dev < DEV_MAX; dev++) {
		struct node *iterator = dev_listeners[dev]->head;
		while (iterator) {
			struct dev_listener *listener = iterator->data;
			struct node *next = iterator->next;
			if (listener->group == group)
				list_remove(dev_listeners[dev], iterator);
			iterator = next;
		}
	}

	if (group + 1 == group_id)
		group_id--;
}

void dev_remove_proc(struct proc *proc)
{
	for (u32 dev = DEV_MIN; dev < DEV_MAX; dev++) {
		struct node *iterator = dev_listeners[dev]->head;
		while (iterator) {
			struct dev_listener *listener = iterator->data;
			struct node *next = iterator->next;
			if (listener->proc == proc)
				list_remove(dev_listeners[dev], iterator);
			iterator = next;
		}
	}
}

CLEAR void dev_add(enum dev_type type, struct dev_dev *dev)
{
	assert(dev_type_valid(type) && !dev_mappings[type]);
	dev_mappings[type] = dev;
}

res dev_poll(u32 *devs)
{
	if (!memory_readable(devs))
		return -EFAULT;

	u32 group = group_id++;

	for (u32 *p = devs; p && memory_readable(p); p++) {
		stac();
		enum dev_type type = *p;
		clac();

		if (!type)
			break;

		struct dev_dev *dev = dev_get(type);
		if (!dev || !dev->read) {
			dev_remove_group(group);
			return -ENOENT;
		}

		if (dev->ready) {
			res ready = dev->ready();
			if (ready == EOK) {
				dev_remove_group(group);
				return type;
			} else if (ready != -EAGAIN) {
				return ready;
			}
		}

		struct dev_listener *listener = malloc(sizeof(*listener));
		listener->group = group;
		listener->proc = proc_current();
		list_add(dev_listeners[type], listener);
	}

	proc_state(proc_current(), PROC_BLOCKED);
	proc_yield();
	return dev_poll(devs);
}

res dev_control(enum dev_type type, u32 request, void *arg1, void *arg2, void *arg3)
{
	struct dev_dev *dev;
	if (!(dev = dev_get(type)) || !dev->control)
		return -ENOENT;

	return dev->control(request, arg1, arg2, arg3);
}

res dev_write(enum dev_type type, const void *buf, u32 offset, u32 count)
{
	if (!memory_readable_range(memory_range(buf, count)))
		return -EFAULT;

	struct dev_dev *dev;
	if (!(dev = dev_get(type)) || !dev->write)
		return -ENOENT;

	return dev->write(buf, offset, count);
}

res dev_read(enum dev_type type, void *buf, u32 offset, u32 count)
{
	if (!memory_writable_range(memory_range(buf, count)))
		return -EFAULT;

	struct dev_dev *dev;
	if (!(dev = dev_get(type)) || !dev->read)
		return -ENOENT;

	if (dev->ready && dev->ready() != EOK)
		return -EAGAIN;

	return dev->read(buf, offset, count);
}

res dev_ready(enum dev_type type)
{
	struct dev_dev *dev;
	if (!(dev = dev_get(type)))
		return -ENOENT;

	if (dev->ready && dev->ready() != EOK)
		return -EAGAIN;

	return EOK;
}

void dev_block(enum dev_type type, struct proc *proc)
{
	assert(dev_type_valid(type));
	struct dev_listener *listener = zalloc(sizeof(*listener));
	listener->group = group_id++;
	listener->proc = proc;
	list_add(dev_listeners[type], listener);
	proc_state(proc_current(), PROC_BLOCKED);
	proc_yield();
}

void dev_unblock(enum dev_type type)
{
	assert(dev_type_valid(type));
	struct node *iterator = dev_listeners[type]->head;
	while (iterator) {
		struct dev_listener *listener = iterator->data;
		struct proc *proc = listener->proc;
		proc_state(proc, PROC_RUNNING);
		struct node *next = iterator->next;
		dev_remove_group(listener->group);
		free(listener);
		iterator = next;
	}

	if (proc_idle())
		proc_yield();
}

void dev_unblock_pid(u32 pid)
{
	for (u32 type = DEV_MIN; type < DEV_MAX; type++) {
		struct node *iterator = dev_listeners[type]->head;
		while (iterator) {
			struct dev_listener *listener = iterator->data;
			struct proc *proc = listener->proc;
			proc_state(proc, PROC_RUNNING);
			struct node *next = iterator->next;
			if (proc->pid == pid) {
				list_remove(dev_listeners[type], iterator);
				free(listener);
			}
			iterator = next;
		}
	}

	if (proc_idle())
		proc_yield();
}

CLEAR void dev_install(void)
{
	for (u32 i = 0; i < DEV_MAX; i++)
		dev_listeners[i] = list_new();

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
	 * Other devices
	 */

	fb_install();
	timer_install();
	logger_install();
	bus_install();
}
