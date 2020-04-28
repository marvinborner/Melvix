#include <syscall.h>

/**
 * DEFINITIONS
 */
DEFN_SYSCALL0(halt, 0);

DEFN_SYSCALL1(write, 1, const char *);

DEFN_SYSCALL1(read, 2, const char *);

DEFN_SYSCALL1(writec, 3, char);

DEFN_SYSCALL0(readc, 4);

DEFN_SYSCALL0(get_pointers, 5);

DEFN_SYSCALL1(alloc, 6, uint32_t);

DEFN_SYSCALL1(free, 7, uint32_t);
