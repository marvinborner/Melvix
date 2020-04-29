#include <syscall.h>

/**
 * DEFINITIONS
 */
DEFN_SYSCALL0(halt, 0);

DEFN_SYSCALL1(exec, 1, char *);

DEFN_SYSCALL1(putch, 2, char *);

DEFN_SYSCALL0(scancode, 3);

DEFN_SYSCALL1(malloc, 4, u32);

DEFN_SYSCALL1(free, 5, u32);

DEFN_SYSCALL0(pointers, 6);