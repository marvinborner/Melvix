// MIT License, Copyright (c) 2020 Marvin Borner
// Syscall implementation

#include <arg.h>
#include <assert.h>
#include <errno.h>
#include <sys.h>

#ifdef USER

/**
 * Definitions
 */

#define ERRIFY(ret)                                                                                \
	if (ret < 0) {                                                                             \
		errno = -ret;                                                                      \
		return -1;                                                                         \
	}                                                                                          \
	errno = 0;                                                                                 \
	return ret

res sys0(enum sys num);
res sys0(enum sys num)
{
	int a;
	__asm__ volatile("int $0x80" : "=a"(a) : "0"(num));
	ERRIFY(a);
}

res sys1(enum sys num, int d1);
res sys1(enum sys num, int d1)
{
	int a;
	__asm__ volatile("int $0x80" : "=a"(a) : "0"(num), "b"((int)d1));
	ERRIFY(a);
}

res sys2(enum sys num, int d1, int d2);
res sys2(enum sys num, int d1, int d2)
{
	int a;
	__asm__ volatile("int $0x80" : "=a"(a) : "0"(num), "b"((int)d1), "c"((int)d2));
	ERRIFY(a);
}

res sys3(enum sys num, int d1, int d2, int d3);
res sys3(enum sys num, int d1, int d2, int d3)
{
	int a;
	__asm__ volatile("int $0x80"
			 : "=a"(a)
			 : "0"(num), "b"((int)d1), "c"((int)d2), "d"((int)d3));
	ERRIFY(a);
}

res sys4(enum sys num, int d1, int d2, int d3, int d4);
res sys4(enum sys num, int d1, int d2, int d3, int d4)
{
	int a;
	__asm__ volatile("int $0x80"
			 : "=a"(a)
			 : "0"(num), "b"((int)d1), "c"((int)d2), "d"((int)d3), "S"((int)d4));
	ERRIFY(a);
}

res sys5(enum sys num, int d1, int d2, int d3, int d4, int d5);
res sys5(enum sys num, int d1, int d2, int d3, int d4, int d5)
{
	int a;
	__asm__ volatile("int $0x80"
			 : "=a"(a)
			 : "0"(num), "b"((int)d1), "c"((int)d2), "d"((int)d3), "S"((int)d4),
			   "D"((int)d5));
	ERRIFY(a);
}

/**
 * Syscalls
 */

res sys_alloc(u32 size, u32 *addr)
{
	u32 id = 0;
	return sys4(SYS_ALLOC, (int)size, (int)addr, (int)&id, 0);
}

res sys_free(void *ptr)
{
	return sys1(SYS_FREE, (int)ptr);
}

res shalloc(u32 size, u32 *addr, u32 *id)
{
	return sys4(SYS_ALLOC, (int)size, (int)addr, (int)id, 1);
}

res shaccess(u32 id, u32 *addr, u32 *size)
{
	return sys3(SYS_SHACCESS, (int)id, (int)addr, (int)size);
}

res read(const char *path, void *buf, u32 offset, u32 count)
{
	return sys4(SYS_READ, (int)path, (int)buf, (int)offset, (int)count);
}

res write(const char *path, const void *buf, u32 offset, u32 count)
{
	return sys4(SYS_WRITE, (int)path, (int)buf, (int)offset, (int)count);
}

res stat(const char *path, struct stat *buf)
{
	return sys2(SYS_STAT, (int)path, (int)buf);
}

res exec(const char *path, ...)
{
	va_list ap;
	int args[4] = { 0 };

	va_start(ap, path);
	for (int i = 0; i < 4; i++)
		args[i] = va_arg(ap, int);
	va_end(ap);

	return sys5(SYS_EXEC, (int)path, args[0], args[1], args[2], args[3]);
}

res dev_poll(enum dev_type *devs)
{
	return sys1(SYS_DEV_POLL, (int)devs);
}

res dev_read(enum dev_type io, void *buf, u32 offset, u32 count)
{
	return sys4(SYS_DEV_READ, (int)io, (int)buf, (int)offset, (int)count);
}

res dev_write(enum dev_type io, const void *buf, u32 offset, u32 count)
{
	return sys4(SYS_DEV_WRITE, (int)io, (int)buf, (int)offset, (int)count);
}

res dev_control(enum dev_type io, ...)
{
	va_list ap;
	int args[4] = { 0 };

	va_start(ap, io);
	for (int i = 0; i < 4; i++)
		args[i] = va_arg(ap, int);
	va_end(ap);

	return sys5(SYS_DEV_CONTROL, (int)io, args[0], args[1], args[2], args[3]);
}

res yield(void)
{
	return sys0(SYS_YIELD);
}

static void atexit_trigger(void);
void exit(s32 status)
{
	atexit_trigger();
	sys1(SYS_EXIT, (int)status);
	while (1)
		yield();
}

res boot(u32 cmd)
{
	return sys2(SYS_BOOT, SYS_BOOT_MAGIC, cmd);
}

/**
 * At exit
 */

#define ATEXIT_MAX 32

static u32 slot = 0;
static void (*funcs[ATEXIT_MAX])(void) = { 0 };

static void atexit_trigger(void)
{
	if (!slot)
		return;

	while (slot-- > 0) {
		if (funcs[slot]) {
			funcs[slot]();
			funcs[slot] = NULL;
		}
	}
}

void atexit(void (*func)(void))
{
	if (slot < ATEXIT_MAX)
		funcs[slot++] = func;
}

#endif
