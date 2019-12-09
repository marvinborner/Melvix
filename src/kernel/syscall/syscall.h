#ifndef MELVIX_SYSCALL_H
#define MELVIX_SYSCALL_H

extern void idt_syscall();

void syscalls_install();

uint32_t sys_write(char *buf);

uint32_t sys_writec(char ch);

uint32_t sys_read(char *buf);

uint32_t sys_readc();

#endif