#ifndef MELVIX_SYSCALL_H
#define MELVIX_SYSCALL_H

extern void idt_syscall();

void syscalls_install();

uint32_t sys_write(unsigned int buf, unsigned int count);

#endif