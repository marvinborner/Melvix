// MIT License, Copyright (c) 2020 Marvin Borner
// Syscall implementation

#include <arg.h>
#include <errno.h>
#include <sys.h>

#if defined(userspace)

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

s32 sys0(enum sys num);
s32 sys0(enum sys num)
{
	int a;
	__asm__ volatile("int $0x80" : "=a"(a) : "0"(num));
	ERRIFY(a);
}

s32 sys1(enum sys num, int d1);
s32 sys1(enum sys num, int d1)
{
	int a;
	__asm__ volatile("int $0x80" : "=a"(a) : "0"(num), "b"((int)d1));
	ERRIFY(a);
}

s32 sys2(enum sys num, int d1, int d2);
s32 sys2(enum sys num, int d1, int d2)
{
	int a;
	__asm__ volatile("int $0x80" : "=a"(a) : "0"(num), "b"((int)d1), "c"((int)d2));
	ERRIFY(a);
}

s32 sys3(enum sys num, int d1, int d2, int d3);
s32 sys3(enum sys num, int d1, int d2, int d3)
{
	int a;
	__asm__ volatile("int $0x80"
			 : "=a"(a)
			 : "0"(num), "b"((int)d1), "c"((int)d2), "d"((int)d3));
	ERRIFY(a);
}

s32 sys4(enum sys num, int d1, int d2, int d3, int d4);
s32 sys4(enum sys num, int d1, int d2, int d3, int d4)
{
	int a;
	__asm__ volatile("int $0x80"
			 : "=a"(a)
			 : "0"(num), "b"((int)d1), "c"((int)d2), "d"((int)d3), "S"((int)d4));
	ERRIFY(a);
}

s32 sys5(enum sys num, int d1, int d2, int d3, int d4, int d5);
s32 sys5(enum sys num, int d1, int d2, int d3, int d4, int d5)
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

void *sys_alloc(u32 size)
{
	return (void *)sys1(SYS_ALLOC, (int)size);
}

u32 shalloc(u32 size)
{
	return (u32)sys1(SYS_SHALLOC, (int)size);
}

void *shaccess(u32 id)
{
	return (void *)sys1(SYS_SHACCESS, (int)id);
}

// TODO: Freeing by ptr + size could be a security risk -> only by address!
void sys_free(void *ptr, u32 size)
{
	sys2(SYS_FREE, (int)ptr, (int)size);
}

void loop(void)
{
	sys0(SYS_LOOP);
}

s32 read(const char *path, void *buf, u32 offset, u32 count)
{
	return sys4(SYS_READ, (int)path, (int)buf, (int)offset, (int)count);
}

s32 write(const char *path, const void *buf, u32 offset, u32 count)
{
	return sys4(SYS_WRITE, (int)path, (int)buf, (int)offset, (int)count);
}

s32 ioctl(const char *path, ...)
{
	va_list ap;
	int args[4] = { 0 };

	va_start(ap, path);
	for (int i = 0; i < 4; i++)
		args[i] = va_arg(ap, int);
	va_end(ap);

	return sys5(SYS_IOCTL, (int)path, args[0], args[1], args[2], args[3]);
}

s32 stat(const char *path, struct stat *buf)
{
	return sys2(SYS_STAT, (int)path, (int)buf);
}

s32 poll(const char **files)
{
	return sys1(SYS_POLL, (int)files);
}

s32 exec(const char *path, ...)
{
	va_list ap;
	int args[4] = { 0 };

	va_start(ap, path);
	for (int i = 0; i < 4; i++)
		args[i] = va_arg(ap, int);
	va_end(ap);

	return sys5(SYS_EXEC, (int)path, args[0], args[1], args[2], args[3]);
}

s32 yield(void)
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

s32 boot(u32 cmd)
{
	return sys2(SYS_BOOT, SYS_BOOT_MAGIC, cmd);
}

u32 time(void)
{
	return (u32)sys0(SYS_TIME);
}

/**
 * At exit
 */

#define ATEXIT_MAX 32

static u32 slot = 0;
static void (*funcs[ATEXIT_MAX])(void) = { 0 };

static void atexit_trigger(void)
{
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
