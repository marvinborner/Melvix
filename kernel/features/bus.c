// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <bus.h>
#include <cpu.h>
#include <crypto.h>
#include <def.h>
#include <errno.h>
#include <io.h>
#include <list.h>
#include <mem.h>
#include <mm.h>
#include <stack.h>
#include <str.h>

// TODO: Consider different hashing method (possible duplicates in crc32)

// Bus conns are always between *two* clients!
struct bus_conn {
	u32 bus; // Bus name hash
	u32 conn;
	struct stack *in; // Queue of bus owner
	struct stack *out; // Queue of other client
};

struct bus_message {
	void *data;
	u32 conn;
	u32 size;
};

struct bus {
	u32 pid; // Owner
	char name[128];
	u32 hash;
};

PROTECTED struct list *bus_list = NULL;
PROTECTED struct list *bus_conns = NULL;
static u32 conn_cnt = 1; // 0 is reserved

static struct bus *bus_find_bus(u32 hash)
{
	struct node *iterator = bus_list->head;
	while (iterator) {
		struct bus *bus = iterator->data;
		if (bus->hash == hash)
			return bus;

		iterator = iterator->next;
	}

	return NULL;
}

static struct bus_conn *bus_find_conn(u32 conn)
{
	struct node *iterator = bus_conns->head;
	while (iterator) {
		struct bus_conn *bus_conn = iterator->data;
		if (bus_conn->conn == conn)
			return bus_conn;

		iterator = iterator->next;
	}

	return NULL;
}

static struct bus *bus_find_owner(u32 pid)
{
	struct node *iterator = bus_list->head;
	while (iterator) {
		struct bus *bus = iterator->data;
		if (bus->pid == pid)
			return bus;

		iterator = iterator->next;
	}

	return NULL;
}

static void bus_add_conn(u32 hash, u32 conn)
{
	struct bus_conn *bus_conn = zalloc(sizeof(*bus_conn));
	bus_conn->bus = hash;
	bus_conn->conn = conn;
	bus_conn->in = stack_new();
	bus_conn->out = stack_new();
	list_add(bus_conns, bus_conn);
}

static res bus_register(const char *name)
{
	if (!memory_readable(name))
		return -EFAULT;

	u32 len = strlen_user(name);
	if (len >= 128)
		return -E2BIG;

	u32 hash = crc32_user(0, name, len);
	if (bus_find_bus(hash))
		return -EBUSY;

	struct bus *bus = zalloc(sizeof(*bus));
	strlcpy_user(bus->name, name, sizeof(bus->name));
	bus->pid = proc_current()->pid;
	bus->hash = hash;
	list_add(bus_list, bus);

	return EOK;
}

static res bus_connect(const char *bus, u32 *conn)
{
	if (!memory_readable(bus) || !memory_writable(conn))
		return -EFAULT;

	u32 len = strlen_user(bus);
	if (len >= 128)
		return -E2BIG;

	u32 hash = crc32_user(0, bus, len);
	if (!bus_find_bus(hash))
		return -ENOENT;

	u32 conn_id = conn_cnt++;
	bus_add_conn(hash, conn_id);
	proc_current()->bus_conn = conn_id;

	stac();
	*conn = conn_id;
	clac();

	return EOK;
}

static res bus_send(u32 conn, void *buf, u32 count)
{
	if (!count)
		return EOK;

	if (!memory_readable(buf))
		return -EFAULT;

	struct bus_conn *bus_conn = bus_find_conn(conn);
	if (!bus_conn)
		return -ENOENT;

	struct bus *bus = bus_find_bus(bus_conn->bus);
	if (!bus)
		return -ENOENT;

	void *data = zalloc(count);
	memcpy_user(data, buf, count);
	struct bus_message *msg = zalloc(sizeof(*msg));
	msg->data = data;
	msg->conn = conn;
	msg->size = count;

	if (bus->pid == proc_current()->pid)
		stack_push_bot(bus_conn->in, msg);
	else
		stack_push_bot(bus_conn->out, msg);

	return count;
}

static res bus_conn_receive(struct bus_conn *bus_conn, void *buf, u32 offset, u32 count)
{
	struct bus *bus = bus_find_bus(bus_conn->bus);
	if (!bus)
		return -ENOENT;

	struct bus_message *msg = NULL;
	if (bus->pid == proc_current()->pid)
		msg = stack_pop(bus_conn->out);
	else
		msg = stack_pop(bus_conn->out);

	if (!msg)
		return -EIO;

	memcpy_user(buf, (u8 *)msg->data + offset, MIN(count, msg->size));
	free(msg->data);
	free(msg);
	return MIN(count, msg->size);
}

static res bus_receive(void *buf, u32 offset, u32 count)
{
	if (!count)
		return EOK;

	if (!memory_readable(buf))
		return -EFAULT;

	struct bus *bus_owner = bus_find_owner(proc_current()->pid);
	struct bus_conn *bus_conn = bus_find_conn(proc_current()->bus_conn);
	if (!bus_owner && !bus_conn)
		return -ENOENT;

	// TODO: Better round-robin

	if (bus_owner) {
		struct node *iterator = bus_conns->head;
		while (iterator) {
			struct bus_conn *sub_bus = iterator->data;
			if (sub_bus->bus == bus_owner->hash)
				return bus_conn_receive(sub_bus, buf, offset, count);

			iterator = iterator->next;
		}
	}

	if (bus_conn)
		return bus_conn_receive(bus_conn, buf, offset, count);

	return -EAGAIN;
}

/**
 * I/O integrations
 */

static res bus_control(u32 request, void *arg1, void *arg2, void *arg3)
{
	UNUSED(arg3);

	switch (request) {
	case IOCTL_BUS_CONNECT: {
		return bus_connect(arg1, arg2);
	}
	case IOCTL_BUS_REGISTER: {
		return bus_register(arg1);
	}
	default: {
		return -EINVAL;
	}
	}
}

static res bus_write(void *buf, u32 offset, u32 count)
{
	if (offset)
		return -EINVAL;

	return bus_send(proc_current()->bus_conn, buf, count);
}

static res bus_read(void *buf, u32 offset, u32 count)
{
	return bus_receive(buf, offset, count);
}

static res bus_conn_ready(struct bus_conn *bus_conn)
{
	struct bus *bus = bus_find_bus(bus_conn->bus);
	if (!bus)
		return -ENOENT;

	u8 ready = 0;
	if (bus->pid == proc_current()->pid)
		ready = !stack_empty(bus_conn->out);
	else
		ready = !stack_empty(bus_conn->out);

	return ready ? EOK : -EAGAIN;
}

static res bus_ready(void)
{
	struct bus *bus_owner = bus_find_owner(proc_current()->pid);
	struct bus_conn *bus_conn = bus_find_conn(proc_current()->bus_conn);
	if (!bus_owner && !bus_conn)
		return -ENOENT;

	// TODO: Better round-robin

	if (bus_owner) {
		struct node *iterator = bus_conns->head;
		while (iterator) {
			struct bus_conn *sub_bus = iterator->data;
			if (sub_bus->bus == bus_owner->hash)
				return bus_conn_ready(sub_bus);

			iterator = iterator->next;
		}
	}

	if (bus_conn)
		return bus_conn_ready(bus_conn);

	return -EAGAIN;
}

CLEAR void bus_install(void)
{
	bus_list = list_new();
	bus_conns = list_new();

	struct io_dev *dev = zalloc(sizeof(*dev));
	dev->control = bus_control;
	dev->read = bus_read;
	dev->ready = bus_ready;
	dev->write = bus_write;
	io_add(IO_BUS, dev);
}
