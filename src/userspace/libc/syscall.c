#include <syscall.h>

/**
 * DEFINITIONS
 */
DEFN_SYSCALL0(halt, 0);

DEFN_SYSCALL1(putch, 1, const char *);

DEFN_SYSCALL0(getch, 2);

DEFN_SYSCALL1(malloc, 3, uint32_t);

DEFN_SYSCALL1(free, 4, uint32_t);