// MIT License, Copyright (c) 2021 Marvin Borner

#include <errno.h>

#include <dev/management.h>
#include <dev/sys.h>

ssize_t sys_dev_read(dev_t type, void *buf, off_t offset, size_t count)
{
	struct dev *dev = dev_get(type);
	if (!dev)
		return -EINVAL;
	if (!dev->exists)
		return -ENODEV;
	if (!dev->read)
		return -EINVAL;
	return dev->read(buf, offset, count);
}

int sys_dev_poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	return 0;
}

int sys_dev_request(dev_t type, dev_req_t request, ...)
{
	struct dev *dev = dev_get(type);
	if (!dev)
		return -EINVAL;
	if (!dev->exists)
		return -ENODEV;
	if (!dev->request)
		return -EINVAL;

	va_list ap;
	va_start(ap, request);
	int result = dev->request(request, ap);
	va_end(ap);

	return result;
}
