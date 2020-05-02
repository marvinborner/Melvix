#include <syscall.h>

/**
 * DEFINITIONS
 */
DEFN_SYSCALL0(halt, 0);

DEFN_SYSCALL1(exit, 1, u32);

DEFN_SYSCALL0(fork, 2);

DEFN_SYSCALL4(read, 3, char *, u32, u32, char *);

DEFN_SYSCALL4(write, 4, char *, u32, u32, char *);

DEFN_SYSCALL1(exec, 5, char *);

DEFN_SYSCALL0(get_pid, 6);

DEFN_SYSCALL1(malloc, 7, u32);

DEFN_SYSCALL1(free, 8, u32);