#include <stdint.h>
#include <common.h>
#include <syscall.h>

/**
 * DEFINITIONS
 */
DEFN_SYSCALL0(halt, SYS_HALT);

DEFN_SYSCALL1(exit, SYS_EXIT, u32);

DEFN_SYSCALL0(fork, SYS_FORK);

DEFN_SYSCALL4(read, SYS_READ, char *, u32, u32, u8 *);

DEFN_SYSCALL4(write, SYS_WRITE, char *, u32, u32, u8 *);

DEFN_SYSCALL1(exec, SYS_EXEC, char *);

DEFN_SYSCALL0(get_pid, SYS_GET_PID);

DEFN_SYSCALL1(malloc, SYS_MALLOC, u32);

DEFN_SYSCALL1(free, SYS_FREE, u32);