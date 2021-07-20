// MIT License, Copyright(c) 2021 Marvin Borner

#ifndef SYS_CALL_H
#define SYS_CALL_H

typedef enum {
	CALL_INVALID,
	CALL_FS_READ,
	CALL_FS_WRITE,
	CALL_FS_STAT,
	CALL_FS_CREATE,
	CALL_DEV_READ,
	CALL_DEV_WRITE,
	CALL_DEV_POLL,
	CALL_DEV_REQUEST,
} syscall_t;

#include <_syscall.h>

#endif
