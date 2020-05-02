#ifndef MELVIX_SYSCALL_H
#define MELVIX_SYSCALL_H

#include <stdint.h>
#include <kernel/interrupts/interrupts.h>

extern void idt_syscall();

void syscalls_install();

uint32_t sys_exit(uint32_t code);

uint32_t sys_fork(struct regs *r);

uint32_t sys_read(char *path, uint32_t offset, uint32_t count, char *buf);

uint32_t sys_write(char *path, uint32_t offset, uint32_t count, char *buf);

uint32_t sys_exec(char *path);

uint32_t sys_get_pid();

uint32_t sys_malloc(uint32_t count);

uint32_t sys_free(uint32_t ptr);

#endif