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
	proc_jump(a);
}

void syscall_init()
{
	idt_set_gate(0x80, (u32)isr128, 0x08, 0x8E);
	isr_install_handler(0x80, syscall_handler);
}
