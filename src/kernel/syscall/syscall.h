#ifndef MELVIX_SYSCALL_H
#define MELVIX_SYSCALL_H

extern void idt_syscall();

void syscalls_install();

uint32_t sys_write(char *buf);

uint32_t sys_writec(char ch);

uint32_t sys_read();

uint32_t sys_readc(char *ch);

uint32_t sys_get_pointers();

uint32_t sys_paging_alloc(uint32_t count);

uint32_t sys_paging_free(uint32_t virt, uint32_t count);

#endif