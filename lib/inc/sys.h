// MIT License, Copyright (c) 2020 Marvin Borner
// Syscall implementation

#ifndef SYS_H
#define SYS_H

enum sys { SYS_LOOP, SYS_MALLOC, SYS_FREE, SYS_EXEC };

int sys0(enum sys num);
int sys1(enum sys num, int d1);
int sys2(enum sys num, int d1, int d2);
int sys3(enum sys num, int d1, int d2, int d3);
int sys4(enum sys num, int d1, int d2, int d3, int d4);
int sys5(enum sys num, int d1, int d2, int d3, int d4, int d5);

#endif
