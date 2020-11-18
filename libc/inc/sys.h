// MIT License, Copyright (c) 2020 Marvin Borner
// Syscall implementation

#ifndef SYS_H
#define SYS_H

#define KEYBOARD_MAGIC 0x555555
#define MOUSE_MAGIC 0xaaaaaa

enum sys {
	SYS_LOOP, // To infinity and beyond (debug)!
	SYS_MALLOC, // Allocate memory
	SYS_FREE, // Free memory
	SYS_READ, // Read file
	SYS_WRITE, // Write to file
	SYS_EXEC, // Execute path
	SYS_EXIT, // Exit current process
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
#define read(path) (void *)sys1(SYS_READ, (int)(path))
#define write(path, buf) sys2(SYS_WRITE, (int)(path), (buf))
#define exec(path, ...) (int)sysv(SYS_EXEC, (int)(path), ##__VA_ARGS__)
#define exit(status)                                                                               \
	{                                                                                          \
		sys1(SYS_EXIT, (int)status);                                                       \
		while (1) {                                                                        \
			yield();                                                                   \
		}                                                                                  \
	}
#define yield() (int)sys0(SYS_YIELD)
#define time() (int)sys0(SYS_TIME)

#define event_register(id) sys1(SYS_REGISTER, (int)(id))
#define event_unregister(id) sys1(SYS_UNREGISTER, (int)(id))

#define msg_send(pid, type, msg) sys3(SYS_SEND, (int)(pid), (int)(type), (int)(msg))
#define msg_receive() (struct message *)sys0(SYS_RECEIVE)
#define getpid() (int)sys0(SYS_GETPID)
static inline struct message *msg_receive_loop()
{
	struct message *msg;
	while (!(msg = msg_receive()))
		yield();
	return msg;
}

#endif
#endif
