#include <syscall.h>

/**
 * DEFINITIONS
 */
DEFN_SYSCALL0(halt, 0);

DEFN_SYSCALL1(write, 1, char *);

DEFN_SYSCALL1(read, 2, char *);

DEFN_SYSCALL1(writec, 3, char *);

DEFN_SYSCALL1(readc, 4, char *);

DEFN_SYSCALL1(paging_alloc, 5, uint32_t);

DEFN_SYSCALL2(paging_free, 6, uint32_t, uint32_t);