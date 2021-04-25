// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <errno.h>
#include <fs.h>
#include <interrupts.h>
#include <io.h>
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

	/* printf("[SYSCALL] %d from %s\n", num, proc_current()->name); */

	switch (num) {
	// Memory operations
	case SYS_ALLOC: {
		r->eax = memory_sys_alloc(proc_current()->page_dir, r->ebx, (u32 *)r->ecx,
					  (u32 *)r->edx, (u8)r->esi);
		break;
	}
	case SYS_FREE: {
		r->eax = memory_sys_free(proc_current()->page_dir, r->ebx);
		break;
	}
	case SYS_SHACCESS: {
		r->eax = memory_sys_shaccess(proc_current()->page_dir, r->ebx, (u32 *)r->ecx,
					     (u32 *)r->edx);
		break;
	}

	// File operations
	case SYS_STAT: {
		r->eax = vfs_stat((char *)r->ebx, (struct stat *)r->ecx);
		break;
	}
	case SYS_READ: {
		r->eax = vfs_read((char *)r->ebx, (void *)r->ecx, r->edx, r->esi);
		break;
	}
	case SYS_WRITE: {
		r->eax = vfs_write((char *)r->ebx, (void *)r->ecx, r->edx, r->esi);
		break;
	}

	// I/O operations
	case SYS_IOPOLL: {
		r->eax = io_poll((void *)r->ebx);
		break;
	}
	case SYS_IOREAD: {
		res ready = io_ready(r->ebx);
		if (ready == -EAGAIN) {
			io_block(r->ebx, proc_current());
		} else if (ready != EOK) {
			r->eax = ready;
			break;
		}
		r->eax = io_read(r->ebx, (void *)r->ecx, r->edx, r->esi);
		break;
	}
	case SYS_IOWRITE: {
		r->eax = io_write(r->ebx, (void *)r->ecx, r->edx, r->esi);
		break;
	}
	case SYS_IOCONTROL: {
		r->eax = io_control(r->ebx, r->ecx, (void *)r->edx, (void *)r->esi, (void *)r->edi);
		break;
	}

	// Process operations
	case SYS_EXEC: {
		char *path = (char *)r->ebx;
		struct proc *proc = proc_make(PROC_PRIV_NONE);
		r->eax = (u32)elf_load(path, proc);
		if (r->eax != EOK) {
			proc_exit(proc, r, -r->eax);
		} else {
			// TODO: Reimplement argc,argv
			proc_stack_push(proc, 0);
			proc_yield();
		}
		break;
	}
	case SYS_EXIT: {
		r->eax = EOK;
		proc_exit(proc_current(), r, (s32)r->ebx);
		break;
	}
	case SYS_YIELD: {
		r->eax = EOK;
		proc_yield();
		break;
	}

	// System operations
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
			__asm__ volatile("cli\nud2");
			break;
		case SYS_BOOT_SHUTDOWN:
			print("Shutting down...\n");
			outw(0xB004, 0x2000);
			outw(0x604, 0x2000);
			outw(0x4004, 0x3400);
			outb(0x64, 0xfe);
			__asm__ volatile("cli\nud2");
			break;
		default:
			r->eax = -EINVAL;
		}
		break;
	}

	case SYS_MIN:
	case SYS_MAX:
		r->eax = -EINVAL;
		break;

	// TODO: Reimplement network functions using VFS
	default: {
		r->eax = -EINVAL;
		printf("Unknown syscall %d!\n", num);
		break;
	}
	}
}

// For kernel syscalls (internal)
static void syscall_special_handler(struct regs *r)
{
	if (RING(r) != 0)
		return;

	scheduler(r);
}

CLEAR void syscall_init(void)
{
	idt_set_gate(0x7f, (u32)isr127, 0x08, 0x8e);
	idt_set_gate(0x80, (u32)isr128, 0x08, 0xee);
	isr_install_handler(0x7f, syscall_special_handler);
	isr_install_handler(0x80, syscall_handler);
}
