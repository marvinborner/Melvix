// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef POLL_H
#define POLL_H

#include <sys/dev.h>

#define POLLIN 0x0001
#define POLLPRI 0x0002
#define POLLOUT 0x0004
#define POLLERR 0x0008
#define POLLHUP 0x0010
#define POLLNVAL 0x0020
#define POLLRDNORM 0x0040
#define POLLRDBAND 0x0080
#define POLLWRBAND 0x0200
#define POLLMSG 0x0400
#define POLLREMOVE 0x1000

struct pollfd {
	dev_t dev;
	short events;
	short revents;
};

typedef unsigned int nfds_t;

#endif
