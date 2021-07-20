// MIT License, Copyright(c) 2021 Marvin Borner

#include <sys/call.h>
#include <unistd.h>

ssize_t fs_read(const char *pathname, void *buf, off_t offset, size_t count)
{
	return syscall(CALL_FS_READ, (syscall_arg_t)pathname, (syscall_arg_t)buf,
		       (syscall_arg_t)offset, (syscall_arg_t)count);
}

ssize_t fs_write(const char *pathname, const void *buf, off_t offset, size_t count)
{
	return syscall(CALL_FS_WRITE, (syscall_arg_t)pathname, (syscall_arg_t)buf,
		       (syscall_arg_t)offset, (syscall_arg_t)count);
}

int fs_stat(const char *pathname, struct stat *statbuf)
{
	return syscall(CALL_FS_STAT, (syscall_arg_t)pathname, (syscall_arg_t)statbuf);
}

int fs_create(const char *pathname)
{
	return syscall(CALL_FS_CREATE, (syscall_arg_t)pathname);
}

ssize_t dev_read(dev_t type, void *buf, off_t offset, size_t count)
{
	return syscall(CALL_DEV_READ, (syscall_arg_t)type, (syscall_arg_t)buf,
		       (syscall_arg_t)offset, (syscall_arg_t)count);
}

ssize_t dev_write(dev_t type, const void *buf, off_t offset, size_t count)
{
	return syscall(CALL_DEV_WRITE, (syscall_arg_t)type, (syscall_arg_t)buf,
		       (syscall_arg_t)offset, (syscall_arg_t)count);
}

int dev_poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	return syscall(CALL_DEV_POLL, (syscall_arg_t)fds, (syscall_arg_t)nfds,
		       (syscall_arg_t)timeout);
}

int dev_request(dev_t type, dev_req_t request, ...)
{
	va_list ap;
	va_start(ap, request);
	int result = syscall(CALL_DEV_REQUEST, (syscall_arg_t)type, (syscall_arg_t)request,
			     (syscall_arg_t)ap);
	va_end(ap);
	return result;
}
