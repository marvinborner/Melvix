// MIT License, Copyright (c) 2020 Marvin Borner
// Syscall implementation

#ifndef SYS_H
#define SYS_H

#include <def.h>

#define KEYBOARD_MAGIC 0x555555
#define MOUSE_MAGIC 0xaaaaaa

#define SYS_BOOT_MAGIC 0x18122002
#define SYS_BOOT_REBOOT 0xeeb007
#define SYS_BOOT_SHUTDOWN 0xdead

enum sys {
	SYS_LOOP, // To infinity and beyond (debug)!
	SYS_ALLOC, // Allocate memory
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
	SYS_NET_OPEN, // Open network socket
	SYS_NET_CLOSE, // Close network socket
	SYS_NET_CONNECT, // Connect to destination
	SYS_NET_SEND, // Send to socket
	SYS_NET_RECEIVE, // Receive data from socket
};

struct event_keyboard {
	u32 magic;
	int press;
	int scancode;
};

struct event_mouse {
	u32 magic;
	int diff_x;
	int diff_y;
	int but1;
	int but2;
	int but3;
};

struct stat {
	u32 dev_id;
	u32 mode;
	u32 uid;
	u32 gid;
	u32 size;
};

#if defined(userspace)

int sys0(enum sys num);
int sys1(enum sys num, int d1);
int sys2(enum sys num, int d1, int d2);
int sys3(enum sys num, int d1, int d2, int d3);
int sys4(enum sys num, int d1, int d2, int d3, int d4);
int sys5(enum sys num, int d1, int d2, int d3, int d4, int d5);
int sysv(enum sys num, ...);

/**
 * Syscall wrappers
 */

#define loop(void) sys0(SYS_LOOP)
#define read(path, buf, offset, count)                                                             \
	(s32) sys4(SYS_READ, (int)(path), (int)(buf), (int)(offset), (int)(count))
#define write(path, buf, offset, count)                                                            \
	(s32) sys4(SYS_WRITE, (int)(path), (int)(buf), (int)(offset), (int)(count))
#define ioctl(path, ...) (s32) sysv(SYS_IOCTL, (int)(path), ##__VA_ARGS__)
#define stat(path, stat) (s32) sys2(SYS_STAT, (int)(path), (int)(stat))
#define poll(files) (s32) sys1(SYS_POLL, (int)(files))
#define exec(path, ...) (s32) sysv(SYS_EXEC, (int)(path), ##__VA_ARGS__)
#define exit(status)                                                                               \
	{                                                                                          \
		sys1(SYS_EXIT, (int)status);                                                       \
		while (1) {                                                                        \
			yield();                                                                   \
		}                                                                                  \
	}
#define boot(cmd) (s32) sys2(SYS_BOOT, SYS_BOOT_MAGIC, cmd)
#define yield(void) (s32) sys0(SYS_YIELD)
#define time(void) (u32) sys0(SYS_TIME)

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
static inline u32 pidof(const char *name)
{
	u32 curr = 1;
	char buf[32] = { 0 }, path[32] = { 0 };
	while (curr < 1000) { // Max pid??
		if (sprintf(path, "/proc/%d/name", curr) > 0 && read(path, buf, 0, 32) > 0)
			if (!strcmp(name, buf))
				return curr;

		curr++;
	}

	return -1;
}

// Simple read wrapper
#include <mem.h>
static inline void *sread(const char *path)
{
	struct stat s = { 0 };
	if (stat(path, &s) != 0 || !s.size)
		return NULL;
	void *buf = malloc(s.size);
	read(path, buf, 0, s.size);
	return buf;
}

#endif
#endif
