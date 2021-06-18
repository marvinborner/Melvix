// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <errno.h>
#include <libgui/msg.h>
#include <print.h>
#include <sys.h>

// TODO: Remove debug assertions?

res msg_connect_bus(const char *bus, u32 *conn)
{
	res ret = dev_control(DEV_BUS, DEVCTL_BUS_CONNECT_BUS, bus, conn);
	/* assert(ret == EOK && *conn); */
	return ret;
}

res msg_connect_conn(u32 conn)
{
	res ret = dev_control(DEV_BUS, DEVCTL_BUS_CONNECT_CONN, conn);
	/* assert(ret == EOK); */
	return ret;
}

res msg_send(enum message_type type, void *data, u32 size)
{
	assert(size >= sizeof(struct message_header));
	struct message_header *header = data;
	header->magic = MSG_MAGIC;
	header->type = type;
	res ret = dev_write(DEV_BUS, (u8 *)data + sizeof(struct bus_header), 0, size);
	/* assert(ret >= EOK); */
	return ret;
}

res msg_receive(void *buf, u32 size)
{
	res ret = dev_read(DEV_BUS, buf, 0, size);
	struct message_header *header = buf;
	if (header->magic == MSG_MAGIC)
		return ret;
	else
		return -EINVAL;
}
