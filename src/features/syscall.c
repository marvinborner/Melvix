// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <interrupts.h>
#include <load.h>
#include <print.h>
#include <proc.h>
#include <str.h>

int i = 0;
void syscall_handler(struct regs *r)
{
	printf("[SYSCALL] %d\n", r->eax);

	struct proc *a = proc_make();
	bin_load(++i ? "/a" : "/b", a);
	strcpy(a->name, "a");
	proc_print();
}

void syscall_init()
{
	idt_set_gate(0x80, (u32)isr128, 0x08, 0x8E);
	isr_install_handler(0x80, syscall_handler);
}
