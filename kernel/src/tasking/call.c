// MIT License, Copyright (c) 2021 Marvin Borner

#include <errno.h>

#include <arch.h>
#include <dev/sys.h>
#include <fs/sys.h>
#include <tasking/call.h>

uintptr_t syscall_handle(syscall_t call, syscall_arg_t arg0, syscall_arg_t arg1, syscall_arg_t arg2,
			 syscall_arg_t arg3, syscall_arg_t arg4)
{
	switch (call) {
	case CALL_FS_READ:
		return sys_fs_read((const char *)arg0, (void *)arg1, arg2, arg3);
	case CALL_FS_WRITE:
		return sys_fs_write((const char *)arg0, (const void *)arg1, arg2, arg3);
	case CALL_FS_STAT:
		return sys_fs_stat((const char *)arg0, (struct stat *)arg1);
	case CALL_FS_CREATE:
		return sys_fs_create((const char *)arg0);
	case CALL_DEV_READ:
		return sys_dev_read(arg0, (void *)arg1, arg2, arg3);
	case CALL_DEV_WRITE:
		return sys_dev_write(arg0, (const void *)arg1, arg2, arg3);
	case CALL_DEV_POLL:
		return sys_dev_poll((struct pollfd *)arg0, arg1, arg2);
	case CALL_DEV_REQUEST:
		return sys_dev_request(arg0, arg1, arg2);
	case CALL_INVALID:
	default:
		return -EINVAL;
	}
}
