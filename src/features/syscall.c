// MIT License, Copyright (c) 2020 Marvin Borner

#include <interrupts.h>
#include <load.h>
#include <print.h>
#include <proc.h>

void syscall_handler(struct regs *r)
{
	printf("[SYSCALL] %d\n", r->eax);

	struct proc *a = proc_make();
	bin_load("/a", a);
}

void syscall_init()
{
	isr_install_handler(0x80, syscall_handler);
}
