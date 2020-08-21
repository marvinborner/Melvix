// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <event.h>
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
	r->eax = 0;
	printf("[SYSCALL] %d: ", num);

	switch (num) {
	case SYS_LOOP: {
		printf("loop\n");
		loop();
		break;
	}
	case SYS_MALLOC: {
		printf("malloc\n");
		r->eax = (u32)malloc(r->eax);
		break;
	}
	case SYS_FREE: {
		printf("free\n");
		free(r->eax);
		break;
	}
	case SYS_READ: {
		printf("read\n");
		r->eax = (u32)read_file((char *)r->ebx);
		break;
	}
	case SYS_WRITE: {
		printf("write\n");
		// TODO: Write ext2 support
		break;
	}
	case SYS_EXEC: {
		printf("exec\n");
		char *path = (char *)r->ebx;
		struct proc *proc = proc_make();
		r->eax = bin_load(path, proc);
		int argc = 3; // TODO: Add argc evaluator
		char **argv = malloc(sizeof(*argv) * (argc + 1));
		argv[0] = (char *)r->ecx;
		argv[1] = (char *)r->edx;
		argv[2] = (char *)r->esi;
		argv[3] = (char *)r->edi;
		argv[4] = NULL;
		((u32 *)proc->regs.useresp)[0] = argc;
		((u32 *)proc->regs.useresp)[1] = (u32)argv;
		if (r->eax)
			proc->state = PROC_ERROR;
		break;
	}
	case SYS_EXIT: {
		printf("exit\n");
		proc_exit(r->ebx);
		break;
	}
	case SYS_MAP: {
		printf("map\n");
		event_map(r->ebx, (u32 *)r->ecx);
		break;
	}
	case SYS_UNMAP: {
		printf("unmap\n");
		event_unmap(r->ebx, (u32 *)r->ecx);
		break;
	}
	default: {
		printf("unknown\n");
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
