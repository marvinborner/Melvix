// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <errno.h>
#include <fs.h>
#include <interrupts.h>
#include <load.h>
#include <mem.h>
#include <mm.h>
#include <net.h>
#include <print.h>
#include <proc.h>
#include <str.h>
#include <sys.h>
#include <syscall.h>
#include <timer.h>

static void syscall_handler(struct regs *r)
{
	enum sys num = r->eax;
	r->eax = 0;

	/* printf("[SYSCALL] %d from %s\n", num, proc_current()->name); */

	switch (num) {
	case SYS_LOOP: {
		loop();
		panic("Fell out of the looping!\n");
		break;
	}
	case SYS_ALLOC: {
		r->eax = (u32)memory_alloc(proc_current()->page_dir, r->ebx,
					   MEMORY_CLEAR | MEMORY_USER);
		break;
	}
	case SYS_FREE: {
		memory_free(proc_current()->page_dir, memory_range(r->ebx, r->ecx));
		break;
	}
	case SYS_STAT: {
		r->eax = (u32)vfs_stat((char *)r->ebx, (struct stat *)r->ecx);
		break;
	}
	case SYS_READ: {
		if (vfs_ready((char *)r->ebx)) {
			r->eax = (u32)vfs_read((char *)r->ebx, (void *)r->ecx, r->edx, r->esi);
		} else {
			s32 wait = vfs_wait((char *)r->ebx, (u32)vfs_read);
			if (wait != 0)
				r->eax = wait;
			else
				proc_yield(r);
		}
		break;
	}
	case SYS_WRITE: {
		r->eax = (u32)vfs_write((char *)r->ebx, (void *)r->ecx, r->edx, r->esi);
		break;
	}
	case SYS_IOCTL: {
		r->eax = (u32)vfs_ioctl((char *)r->ebx, r->ecx, (void *)r->edx, (void *)r->esi,
					(void *)r->edi);
		break;
	}
	case SYS_POLL: {
		s32 ret = vfs_poll((const char **)r->ebx);
		r->eax = ret;
		if (ret == PROC_MAX_WAIT_IDS + 1)
			proc_yield(r);
		break;
	}
	case SYS_EXEC: {
		char *path = (char *)r->ebx;
		struct proc *proc = proc_make(PROC_PRIV_NONE);
		r->eax = (u32)elf_load(path, proc);
		if (r->eax != 0)
			proc_exit(proc, -r->eax);
		// TODO: Reimplement argc,argv
		proc_stack_push(proc, 0);
		proc_yield(r);
		break;
	}
	case SYS_EXIT: {
		proc_exit(proc_current(), (int)r->ebx);
		break;
	}
	case SYS_BOOT: { // TODO: Move
		if (r->ebx != SYS_BOOT_MAGIC) {
			r->eax = -EINVAL;
			break;
		}
		if (!proc_super()) {
			r->eax = -EACCES;
		}
		switch (r->ecx) {
		case SYS_BOOT_REBOOT:
			print("Rebooting...\n");
			outb(0x64, 0xfe);
			__asm__ volatile("ud2");
			break;
		case SYS_BOOT_SHUTDOWN:
			print("Shutting down...\n");
			outw(0xB004, 0x2000);
			outw(0x604, 0x2000);
			outw(0x4004, 0x3400);
			outb(0x64, 0xfe);
			__asm__ volatile("ud2");
			break;
		default:
			r->eax = -EINVAL;
		}
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
	// TODO: Reimplement network functions using VFS
	case SYS_NET_OPEN:
	case SYS_NET_CLOSE:
	case SYS_NET_CONNECT:
	case SYS_NET_SEND:
	case SYS_NET_RECEIVE:
#if 0
	case SYS_NET_OPEN: {
		r->eax = (int)net_open(r->ebx);
		break;
	}
	case SYS_NET_CLOSE: {
		struct socket *s = (void *)r->ebx;
		int status = net_close(s);
		if (!status) {
			proc_yield(r);
			return;
		}
		r->eax = net_close(s);
		break;
	}
	case SYS_NET_CONNECT: {
		struct socket *s = (void *)r->ebx;
		if (s->state == S_CONNECTED)
			r->eax = 1;
		else if (s->state == S_FAILED || s->state == S_CLOSED)
			r->eax = 0;
		else if (s->state == S_OPEN)
			r->eax = net_connect(s, r->ecx, r->edx);
		break;
	}
	case SYS_NET_SEND: {
		net_send((void *)r->ebx, (void *)r->ecx, r->edx);
		break;
	}
	case SYS_NET_RECEIVE: {
		r->eax = net_receive((void *)r->ebx, (void *)r->ecx, r->edx);
		break;
	}
#endif
	default: {
		printf("Unknown syscall %d!\n", num);
		break;
	}
	}
}

void syscall_init(void)
{
	idt_set_gate(0x80, (u32)isr128, 0x08, 0x8E);
	isr_install_handler(0x80, syscall_handler);
}
