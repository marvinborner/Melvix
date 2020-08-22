// MIT License, Copyright (c) 2020 Marvin Borner
// Syscall implementation

#ifndef SYS_H
#define SYS_H

enum sys {
	SYS_LOOP,
	SYS_MALLOC,
	SYS_FREE,
	SYS_READ,
	SYS_WRITE,
	SYS_EXEC,
	SYS_EXIT,
	SYS_MAP,
	SYS_UNMAP,
	SYS_RESOLVE
};
enum event { EVENT_KEYBOARD, EVENT_MOUSE };

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
#define read(path) (void *)sys1(SYS_READ, (int)path)
#define write(path, buf) sys2(SYS_WRITE, (int)path, buf)
#define exec(path, ...) sysv(SYS_EXEC, (int)path, ##__VA_ARGS__)
#define exit(status)                                                                               \
	{                                                                                          \
		sys1(SYS_EXIT, (int)status);                                                       \
		while (1) {                                                                        \
		}                                                                                  \
	}
#define event_map(id, func) sys2(SYS_MAP, (int)id, (int)func)
#define event_unmap(id, func) sys2(SYS_UNMAP, (int)id, (int)func)
#define event_resolve() sys0(SYS_RESOLVE) // TODO: Find method making event_resolve obsolete

#endif
#endif
