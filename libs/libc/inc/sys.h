// MIT License, Copyright (c) 2020 Marvin Borner
// Syscall implementation

#ifndef SYS_H
#define SYS_H

#include <def.h>
#include <errno.h>

#define KEYBOARD_MAGIC 0x555555
#define MOUSE_MAGIC 0xaaaaaa

#define SYS_BOOT_MAGIC 0x18122002
#define SYS_BOOT_REBOOT 0xeeb007
#define SYS_BOOT_SHUTDOWN 0xdead

enum sys {
	SYS_LOOP, // To infinity and beyond (debug)!
	SYS_ALLOC, // Allocate memory
	SYS_SHACCESS, // Access shared memory
	SYS_FREE, // Free memory
	SYS_STAT, // Get file information
	SYS_READ, // Read file
	SYS_WRITE, // Write to file
	SYS_IOCTL, // Interact with a file/device
	SYS_POLL, // Wait for multiple files
	SYS_EXEC, // Execute path
	SYS_EXIT, // Exit current process
	SYS_BOOT, // Boot functions (e.g. reboot/shutdown)
	SYS_YIELD, // Switch to next process
	SYS_TIME, // Get kernel time
	/* SYS_NET_OPEN, // Open network socket */
	/* SYS_NET_CLOSE, // Close network socket */
	/* SYS_NET_CONNECT, // Connect to destination */
	/* SYS_NET_SEND, // Send to socket */
	/* SYS_NET_RECEIVE, // Receive data from socket */
};

struct event_keyboard {
	u32 magic;
	u32 scancode;
	u8 press;
};

struct event_mouse {
	u32 magic;
	s32 diff_x;
	s32 diff_y;
	u8 but1;
	u8 but2;
	u8 but3;
};

struct stat {
	u32 dev_id;
	u32 mode;
	u32 uid;
	u32 gid;
	u32 size;
};

#ifdef USER

/**
 * Syscall wrappers
 */

void loop(void);
void exit(s32 status) NORETURN;
res read(const char *path, void *buf, u32 offset, u32 count) NONNULL;
res write(const char *path, const void *buf, u32 offset, u32 count) NONNULL;
res ioctl(const char *path, ...) NONNULL;
res stat(const char *path, struct stat *buf) NONNULL;
res poll(const char **files) NONNULL;
res exec(const char *path, ...) ATTR((nonnull(1))) SENTINEL;
res yield(void);
res boot(u32 cmd);
u32 time(void);

res sys_alloc(u32 size, u32 *addr) NONNULL;
res sys_free(void *ptr) NONNULL;
res shalloc(u32 size, u32 *addr, u32 *id) NONNULL;
res shaccess(u32 id, u32 *addr, u32 *size) NONNULL;

static inline u32 getpid(void)
{
	static u32 buf = 0;
	if (buf)
		return buf;
	read("/proc/self/pid", &buf, 0, sizeof(buf));
	return buf;
}

#include <print.h>
#include <str.h>
NONNULL static inline u32 pidof(const char *name)
{
	u32 curr = 1;
	char buf[32] = { 0 }, path[32] = { 0 };
	while (curr < 1000) { // Max pid??
		if (snprintf(path, sizeof(buf), "/proc/%d/name", curr) > 0 &&
		    read(path, buf, 0, 32) > 0)
			if (!strcmp(name, buf))
				return curr;

		curr++;
	}

	return -1;
}

// Simple read wrapper
#include <mem.h>
NONNULL static inline void *sread(const char *path)
{
	struct stat s = { 0 };
	if (stat(path, &s) != 0 || !s.size)
		return NULL;
	void *buf = malloc(s.size);
	read(path, buf, 0, s.size);
	return buf;
}

/**
 * At exit
 */

void atexit(void (*func)(void));

#endif
#endif
