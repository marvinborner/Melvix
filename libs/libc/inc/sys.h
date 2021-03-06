// MIT License, Copyright (c) 2020 Marvin Borner
// Syscall implementation

#ifndef SYS_H
#define SYS_H

#include <def.h>
#include <errno.h>
#include <vec.h>

#define KEYBOARD_MAGIC 0x555555
#define MOUSE_MAGIC 0xaaaaaa

#define SYS_BOOT_MAGIC 0x18122002
#define SYS_BOOT_REBOOT 0xeeb007
#define SYS_BOOT_SHUTDOWN 0xdead

enum sys {
	SYS_MIN,
	SYS_ALLOC, // Allocate memory
	SYS_SHACCESS, // Access shared memory
	SYS_FREE, // Free memory
	SYS_READ, // Read file (non-blocking)
	SYS_WRITE, // Write to file
	SYS_STAT, // Get file information
	SYS_EXEC, // Execute path
	SYS_DEV_POLL, // Block proc until device is ready
	SYS_DEV_READ, // Read data from device (blocking)
	SYS_DEV_WRITE, // Write data to device
	SYS_DEV_CONTROL, // Interact with an I/O device
	SYS_EXIT, // Exit current process
	SYS_BOOT, // Boot functions (e.g. reboot/shutdown)
	SYS_YIELD, // Switch to next process
	SYS_MAX,
};

enum dev_type {
	DEV_MIN,
	DEV_LOGGER,
	DEV_FRAMEBUFFER,
	DEV_NETWORK,
	DEV_KEYBOARD,
	DEV_MOUSE,
	DEV_TIMER,
	DEV_BUS,
	DEV_MAX,
};

// Device control declarations
#define DEVCTL_FB_GET 0
#define DEVCTL_TIMER_SLEEP 0
#define DEVCTL_BUS_CONNECT_BUS 0
#define DEVCTL_BUS_CONNECT_CONN 1
#define DEVCTL_BUS_REGISTER 2

struct fb_generic {
	u16 bpp;
	u16 pitch;
	u16 width;
	u16 height;
	u8 *fb;
};

struct bus_header {
	u32 conn;
	// Data starts here
};

struct event_keyboard {
	u32 magic;
	u32 scancode;
	u8 press;
};

struct event_mouse {
	u32 magic;
	vec2 pos;
	u8 rel; // 1 rel, 0 abs
	s8 scroll; // Dir: -1 neg, +1 pos
	struct {
		u8 left : 1;
		u8 right : 1;
		u8 middle : 1;
	} but;
};

struct timer {
	u32 rtc;
	struct {
		u32 user;
		u32 kernel;
	} ticks;
	u32 time;
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

void exit(s32 status) NORETURN;
res read(const char *path, void *buf, u32 offset, u32 count) NONNULL;
res write(const char *path, const void *buf, u32 offset, u32 count) NONNULL;
res stat(const char *path, struct stat *buf) NONNULL;
res exec(const char *path, ...) ATTR((nonnull(1))) SENTINEL;

res dev_poll(enum dev_type *devs) NONNULL;
res dev_read(enum dev_type dev, void *buf, u32 offset, u32 count) NONNULL;
res dev_write(enum dev_type dev, const void *buf, u32 offset, u32 count) NONNULL;
res dev_control(enum dev_type dev, ...);

res yield(void);
res boot(u32 cmd);

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
