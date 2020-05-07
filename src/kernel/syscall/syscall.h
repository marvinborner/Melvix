#ifndef MELVIX_SYSCALL_H
#define MELVIX_SYSCALL_H

#include <interrupts/interrupts.h>
#include <stdint.h>

extern void idt_syscall();

void syscalls_install();

u32 sys_exit(u32 code);

u32 sys_fork(struct regs *r);

u32 sys_read(char *path, u32 offset, u32 count, u8 *buf);

u32 sys_write(char *path, u32 offset, u32 count, u8 *buf);

u32 sys_exec(char *path);

u32 sys_wait(u32 pid, u32 *status, u32 options);

u32 sys_get_pid();

u32 sys_malloc(u32 count);

u32 sys_free(u32 ptr);

u32 sys_get(u32 id);

u32 sys_map(u32 id, u8 *function);

#endif