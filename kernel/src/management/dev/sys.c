// MIT License, Copyright (c) 2021 Marvin Borner

#include <management/dev/sys.h>

res sys_dev_write(enum dev_type type, const void *buf, u32 offset, u32 count)
{
	struct dev *dev = dev_get(type);
	if (!dev)
		return -EINVAL;
	if (!dev->exists)
		return -ENODEV;
	if (!dev->write)
		return -EINVAL;
	return dev->write(buf, offset, count);
}

res sys_dev_read(enum dev_type type, void *buf, u32 offset, u32 count)
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

res sys_dev_poll(struct dev_poll *fds, u32 nfds, int timeout)
{
	UNUSED(fds);
	UNUSED(nfds);
	UNUSED(timeout);
	return 0;
}

res sys_dev_request(enum dev_type type, u32 request, ...)
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
