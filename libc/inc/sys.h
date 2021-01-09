// MIT License, Copyright (c) 2020 Marvin Borner
// Syscall implementation

#ifndef SYS_H
#define SYS_H

#include <def.h>

#define KEYBOARD_MAGIC 0x555555
#define MOUSE_MAGIC 0xaaaaaa

enum sys {
	SYS_LOOP, // To infinity and beyond (debug)!
	SYS_MALLOC, // Allocate memory
	SYS_FREE, // Free memory
	SYS_STAT, // Get file information
	SYS_READ, // Read file
	SYS_WRITE, // Write to file
	SYS_EXEC, // Execute path
	SYS_EXIT, // Exit current process // TODO: Free all memory of process
	SYS_YIELD, // Switch to next process
	SYS_TIME, // Get kernel time
	SYS_REGISTER, // Register for event
	SYS_UNREGISTER, // Unregister event
	SYS_SEND, // Send message to process
	SYS_RECEIVE, // Receive message (non-blocking/sync)
	SYS_GETPID, // Get the process ID
	SYS_NET_OPEN, // Open network socket
	SYS_NET_CLOSE, // Close network socket
	SYS_NET_CONNECT, // Connect to destination
	SYS_NET_SEND, // Send to socket
	SYS_NET_RECEIVE, // Receive data from socket
};

enum event_type { EVENT_KEYBOARD, EVENT_MOUSE, EVENT_MAX };

struct message {
	int src;
	int type;
	void *data;
};

struct event_keyboard {
	int magic;
	int press;
	int scancode;
};

struct event_mouse {
	int magic;
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
 * Wrappers
 */

#define loop() sys0(SYS_LOOP)
#define stat(path, stat) (u32) sys2(SYS_STAT, (int)(path), (int)(stat))
#define read(path, buf, offset, count)                                                             \
	(u32) sys4(SYS_READ, (int)(path), (int)(buf), (int)(offset), (int)(count))
#define write(path, buf, offset, count)                                                            \
	(u32) sys4(SYS_WRITE, (int)(path), (int)(buf), (int)(offset), (int)(count))
#define exec(path, ...) (int)sysv(SYS_EXEC, (int)(path), ##__VA_ARGS__)
#define exit(status)                                                                               \
	{                                                                                          \
		sys1(SYS_EXIT, (int)status);                                                       \
		while (1) {                                                                        \
			yield();                                                                   \
		}                                                                                  \
	}
#define yield() (int)sys0(SYS_YIELD)
#define time() (u32) sys0(SYS_TIME)

#define event_register(id) sys1(SYS_REGISTER, (int)(id))
#define event_unregister(id) sys1(SYS_UNREGISTER, (int)(id))

#define msg_send(pid, type, msg) sys3(SYS_SEND, (int)(pid), (int)(type), (int)(msg))
#define msg_receive(buf) (struct message *)sys1(SYS_RECEIVE, (int)(buf))
#define getpid() (int)sys0(SYS_GETPID)
static inline struct message *msg_receive_loop(struct message *buf)
{
	while (!msg_receive(&buf))
		yield();
	return buf;
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
