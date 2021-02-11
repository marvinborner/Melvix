// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <msg.h>
#include <print.h>
#include <sys.h>

int msg_send(u32 pid, enum message_type type, void *data)
{
	assert((signed)pid != -1);
	char path[32] = { 0 };
	sprintf(path, "/proc/%d/msg", pid);
	struct message msg = { .magic = MSG_MAGIC, .src = getpid(), .type = type, .data = data };
	return write(path, &msg, 0, sizeof(msg));
}

int msg_receive(struct message *msg)
{
	int ret = read("/proc/self/msg", msg, 0, sizeof(*msg));
	assert(msg->magic == MSG_MAGIC);
	if (msg->magic == MSG_MAGIC && ret == sizeof(*msg))
		return ret;
	else
		return -1;
}
