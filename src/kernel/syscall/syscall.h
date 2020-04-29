#ifndef MELVIX_SYSCALL_H
#define MELVIX_SYSCALL_H

#include <stdint.h>

extern void idt_syscall();

void syscalls_install();

uint32_t sys_exec(char *path);

uint32_t sys_putch(char ch);

uint32_t sys_scancode();

uint32_t sys_malloc(uint32_t count);

uint32_t sys_free(uint32_t ptr);

uint32_t sys_pointers();

#endif