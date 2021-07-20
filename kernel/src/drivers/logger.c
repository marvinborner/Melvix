// MIT License, Copyright (c) 2021 Marvin Borner

#include <arch.h>
#include <dev/management.h>
#include <drivers/logger.h>
#include <errno.h>

static ssize_t logger_write(const void *buf, off_t offset, size_t count)
{
	arch_log((const char *)buf + offset, count);
	return 0;
}

static int logger_request(dev_req_t request, va_list ap)
{
	UNUSED(request);
	UNUSED(ap);
	return -EINVAL;
}

static void logger_enable(void)
{
}

static void logger_disable(void)
{
}

PROTECTED static struct dev dev = {
	.type = DEV_LOGGER,
	.read = NULL,
	.write = logger_write,
	.request = logger_request,
	.enable = logger_enable,
	.disable = logger_disable,
};

void logger_init(void)
{
	dev_add(&dev);
}
