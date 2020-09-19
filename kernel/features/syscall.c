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
#include <timer.h>

void syscall_handler(struct regs *r)
{
	enum sys num = r->eax;
	r->eax = 0;

	/* printf("[SYSCALL] %d from %s\n", num, proc_current()->name); */

	switch (num) {
	case SYS_LOOP: {
		loop();
		break;
	}
	case SYS_MALLOC: {
		r->eax = (u32)malloc(r->ebx);
		break;
	}
	case SYS_FREE: {
		free((void *)r->ebx);
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
			proc_exit(proc, r->eax);
		break;
	}
	case SYS_EXIT: {
		proc_exit(proc_current(), r->ebx);
		break;
	}
	case SYS_YIELD: {
		proc_yield(r);
		break;
	}
	case SYS_TIME: {
		r->eax = timer_get();
		break;
	}
	case SYS_REGISTER: {
		event_register(r->ebx, proc_current());
		break;
	}
	case SYS_UNREGISTER: {
		event_unregister(r->ebx, proc_current());
		break;
	}
	case SYS_SEND: {
		proc_send(proc_current(), proc_from_pid(r->ebx), r->ecx, (void *)r->edx);
		break;
	}
	case SYS_RECEIVE: {
		struct proc_message *msg = proc_receive(proc_current());
		r->eax = (u32)(msg ? msg->msg : NULL);
		break;
	}
	case SYS_GETPID: {
		r->eax = proc_current()->pid;
		break;
	}
	default: {
		print("Unknown syscall!\n");
		break;
	}
	}
}

void syscall_init()
{
	idt_set_gate(0x80, (u32)isr128, 0x08, 0x8E);
	isr_install_handler(0x80, syscall_handler);
}
