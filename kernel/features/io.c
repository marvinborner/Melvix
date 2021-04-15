// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <fb.h>
#include <interrupts.h>
#include <io.h>
#include <list.h>
#include <mm.h>
#include <proc.h>
#include <ps2.h>
#include <rand.h>
#include <str.h>
#include <timer.h>

PROTECTED static struct io_dev *io_mappings[IO_MAX] = { 0 };
PROTECTED static struct list *io_listeners[IO_MAX] = { 0 };

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

CLEAR void io_add(enum io_type io, struct io_dev *dev)
{
	assert(io_type_valid(io) && !io_mappings[io]);
	io_mappings[io] = dev;
}

res io_poll(u32 *devs)
{
	if (!memory_readable(devs))
		return -EFAULT;

	for (u32 *p = devs; p && memory_readable(p) && *p; p++) {
		if (!io_get(*p))
			return -ENOENT;
	}

	return -EFAULT;
}

res io_control(enum io_type io, u32 request, void *arg1, void *arg2, void *arg3)
{
	struct io_dev *dev;
	if (!(dev = io_get(io)) || !dev->control)
		return -ENOENT;

	return dev->control(request, arg1, arg2, arg3);
}

res io_write(enum io_type io, void *buf, u32 offset, u32 count)
{
	if (!memory_readable(buf))
		return -EFAULT;

	struct io_dev *dev;
	if (!(dev = io_get(io)) || !dev->write)
		return -ENOENT;

	return dev->write(buf, offset, count);
}

res io_read(enum io_type io, void *buf, u32 offset, u32 count)
{
	if (!memory_readable(buf))
		return -EFAULT;

	struct io_dev *dev;
	if (!(dev = io_get(io)) || !dev->read)
		return -ENOENT;

	if (dev->ready && !dev->ready())
		return -EAGAIN;

	return dev->read(buf, offset, count);
}

void io_block(enum io_type io, struct proc *proc, struct regs *r)
{
	assert(r->eax == SYS_IOREAD);
	assert(io_type_valid(io));
	list_add(io_listeners[io], proc);
	proc_state(proc_current(), PROC_BLOCKED);
	proc_yield(r);
}

void io_unblock(enum io_type io)
{
	struct page_dir *dir_bak;
	memory_backup_dir(&dir_bak);

	assert(io_type_valid(io));
	struct node *iterator = io_listeners[io]->head;
	while (iterator) {
		struct proc *proc = iterator->data;
		struct regs *r = &proc->regs;

		memory_switch_dir(proc->page_dir);
		r->eax = io_read(r->ebx, (void *)r->ecx, r->edx, r->esi);
		memory_switch_dir(dir_bak);

		proc_state(proc, PROC_RUNNING);
		struct node *next = iterator->next;
		list_remove(io_listeners[io], iterator);
		iterator = next;
	}
}

CLEAR void io_install(struct boot_info *boot)
{
	for (u32 i = 0; i < IO_MAX; i++)
		io_listeners[i] = list_new();

	ps2_detect();

	u8 ps2_keyboard = ps2_keyboard_detect();
	if (ps2_keyboard != U8_MAX) {
		ps2_keyboard_install(ps2_keyboard);
	}

	u8 ps2_mouse = ps2_mouse_detect();
	if (ps2_mouse != U8_MAX) {
		ps2_mouse_install(ps2_mouse);
	}

	timer_install();
	fb_install(boot->vid);
}
