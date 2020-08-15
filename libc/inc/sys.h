// MIT License, Copyright (c) 2020 Marvin Borner
// Syscall implementation

#ifndef SYS_H
#define SYS_H

enum sys { SYS_LOOP, SYS_MALLOC, SYS_FREE, SYS_READ, SYS_WRITE, SYS_EXEC, SYS_EXIT };

#if defined(userspace)

int sys0(enum sys num);
int sys1(enum sys num, int d1);
int sys2(enum sys num, int d1, int d2);
int sys3(enum sys num, int d1, int d2, int d3);
int sys4(enum sys num, int d1, int d2, int d3, int d4);
int sys5(enum sys num, int d1, int d2, int d3, int d4, int d5);

/**
 * Wrappers
 */

#define loop() sys0(SYS_LOOP)
#define read(path) (void *)sys1(SYS_READ, (int)path)
#define write(path, buf) sys2(SYS_WRITE, (int)path, buf)
#define exec(path) sys1(SYS_EXEC, (int)path)
#define exit()                                                                                     \
	sys0(SYS_EXIT);                                                                            \
	while (1) {                                                                                \
	}

#endif
#endif
