// MIT License, Copyright (c) 2020 Marvin Borner
// Syscall implementation

#ifndef SYS_H
#define SYS_H

enum sys {
	SYS_LOOP, // To infinity and beyond (debug)!
	SYS_MALLOC, // Allocate memory
	SYS_FREE, // Free memory
	SYS_READ, // Read file
	SYS_WRITE, // Write to file
	SYS_EXEC, // Execute path
	SYS_EXIT, // Exit current process
	SYS_TIME, // Get kernel time
	SYS_MAP, // Map event to function
	SYS_UNMAP, // Unmap event
	SYS_RESOLVE, // Resolve event (needs to be executed at end of function)
	SYS_SEND, // Send message to process
	SYS_RECEIVE // Receive message (non-blocking/sync)
};
enum event { EVENT_KEYBOARD, EVENT_MOUSE };
enum message_type { MSG_NEW_WINDOW };

struct message {
	int src;
	enum message_type type;
	void *data;
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
		}                                                                                  \
	}
#define time() (int)sys0(SYS_TIME)

#define event_map(id, func) sys2(SYS_MAP, (int)(id), (int)(func))
#define event_unmap(id, func) sys2(SYS_UNMAP, (int)(id), (int)(func))
#define event_resolve() sys0(SYS_RESOLVE) // TODO: Find method making event_resolve obsolete

#define msg_send(pid, type, msg) sys3(SYS_SEND, (int)(pid), (int)(type), (int)(msg))
#define msg_receive() (struct message *)sys0(SYS_RECEIVE)
static inline struct message *msg_receive_loop()
{
	struct message *msg;
	while (!(msg = msg_receive()))
		;
	return msg;
}

#endif
#endif
