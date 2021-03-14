// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <msg.h>
#include <print.h>
#include <sys.h>

int msg_send(u32 pid, enum message_type type, void *data, u32 size)
{
	assert((signed)pid != -1 && size >= sizeof(struct message_header));
	char path[32] = { 0 };
	sprintf(path, "/proc/%d/msg", pid);
	struct message_header *header = data;
	header->magic = MSG_MAGIC;
	header->src = getpid();
	header->type = type;
	return write(path, data, 0, size);
}

int msg_receive(void *buf, u32 size)
{
	int ret = read("/proc/self/msg", buf, 0, size);
	struct message_header *header = buf;
	if (header->magic == MSG_MAGIC)
		return ret;
	else
		return -1;
}
