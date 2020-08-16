// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <fs.h>
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
	case SYS_READ: {
		r->eax = (u32)read_file((char *)r->ebx);
		break;
	}
	case SYS_WRITE: {
		// TODO: Write ext2 support
		break;
	}
	case SYS_EXEC: {
		char *path = (char *)r->ebx;
		struct proc *proc = proc_make();
		((u32 *)proc->regs.esp)[0] = r->ecx;
		((u32 *)proc->regs.esp)[1] = r->edx;
		((u32 *)proc->regs.esp)[2] = r->esi;
		((u32 *)proc->regs.esp)[3] = r->edi;
		r->eax = bin_load(path, proc);
		break;
	}
	case SYS_EXIT: {
		proc_exit();
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
