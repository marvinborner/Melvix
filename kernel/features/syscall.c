// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <interrupts.h>
#include <load.h>
#include <mem.h>
#include <print.h>
#include <proc.h>
#include <str.h>
#include <sys.h>

void syscall_handler(struct regs *r)
{
	enum sys num = r->eax;
	printf("[SYSCALL] %d\n", num);

	switch (num) {
	case SYS_LOOP: {
		loop();
		break;
	}
	case SYS_MALLOC: {
		r->eax = (u32)malloc(r->eax);
		break;
	}
	case SYS_FREE: {
		free(r->eax);
		break;
	}
	case SYS_EXEC: {
		char *path = (char *)r->ebx;
		struct proc *proc = proc_make();
		bin_load(path, proc);
		proc_print();
		break;
	}
	default: {
		printf("Unknown syscall!\n");
		loop();
		break;
	}
	}
}

void syscall_init()
{
	idt_set_gate(0x80, (u32)isr128, 0x08, 0x8E);
	isr_install_handler(0x80, syscall_handler);
}
