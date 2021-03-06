// MIT License, Copyright (c) 2020 Marvin Borner

#include <dev.h>
#include <drivers/cpu.h>
#include <drivers/int.h>
#include <errno.h>
#include <fs.h>
#include <load.h>
#include <mem.h>
#include <mm.h>
#include <print.h>
#include <proc.h>
#include <str.h>
#include <sys.h>
#include <syscall.h>
#include <timer.h>

static u32 syscall_handler(u32 esp)
{
	struct int_frame_user *frame = (struct int_frame_user *)esp;
	enum sys num = frame->eax;

#if DEBUG_SYSCALLS
	printf("[SYSCALL] %d from %s\n", num, proc_current()->name);
#endif

	switch (num) {
	// Memory operations
	case SYS_ALLOC: {
		frame->eax = memory_sys_alloc(proc_current()->page_dir, frame->ebx,
					      (u32 *)frame->ecx, (u32 *)frame->edx, (u8)frame->esi);
		break;
	}
	case SYS_FREE: {
		frame->eax = memory_sys_free(proc_current()->page_dir, frame->ebx);
		break;
	}
	case SYS_SHACCESS: {
		frame->eax = memory_sys_shaccess(proc_current()->page_dir, frame->ebx,
						 (u32 *)frame->ecx, (u32 *)frame->edx);
		break;
	}

	// File operations
	case SYS_STAT: {
		frame->eax = vfs_stat((char *)frame->ebx, (struct stat *)frame->ecx);
		break;
	}
	case SYS_READ: {
		frame->eax =
			vfs_read((char *)frame->ebx, (void *)frame->ecx, frame->edx, frame->esi);
		break;
	}
	case SYS_WRITE: {
		frame->eax =
			vfs_write((char *)frame->ebx, (void *)frame->ecx, frame->edx, frame->esi);
		break;
	}

	// Device operations
	case SYS_DEV_POLL: {
		frame->eax = dev_poll((void *)frame->ebx);
		break;
	}
	case SYS_DEV_READ: {
		// TODO: This could be done more elegant (e.g. with better bus design)
		res ready;
		while ((ready = dev_ready(frame->ebx)) == -EAGAIN)
			dev_block(frame->ebx, proc_current());
		if (ready != EOK) {
			frame->eax = ready;
			break;
		}
		frame->eax = dev_read(frame->ebx, (void *)frame->ecx, frame->edx, frame->esi);
		break;
	}
	case SYS_DEV_WRITE: {
		frame->eax = dev_write(frame->ebx, (void *)frame->ecx, frame->edx, frame->esi);
		break;
	}
	case SYS_DEV_CONTROL: {
		frame->eax = dev_control(frame->ebx, frame->ecx, (void *)frame->edx,
					 (void *)frame->esi, (void *)frame->edi);
		break;
	}

	// Process operations
	case SYS_EXEC: {
		char *path = (char *)frame->ebx;
		struct proc *proc = proc_make(PROC_PRIV_NONE);
		frame->eax = (u32)elf_load(path, proc);
		if (frame->eax != EOK)
			panic("NOT IMPLEMENTED\n"); // TODO: Implement exec path/etc verify
		else
			proc_yield();
		break;
	}
	case SYS_EXIT: {
		frame->eax = EOK;
		proc_exit((s32)frame->ebx);
		break;
	}
	case SYS_YIELD: {
		frame->eax = EOK;
		proc_yield();
		break;
	}

	// System operations
	case SYS_BOOT: { // TODO: Move
		if (frame->ebx != SYS_BOOT_MAGIC) {
			frame->eax = -EINVAL;
			break;
		}
		if (!proc_super()) {
			frame->eax = -EACCES;
		}
		switch (frame->ecx) {
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
			frame->eax = -EINVAL;
		}
		break;
	}

	case SYS_MIN:
	case SYS_MAX:
		frame->eax = -EINVAL;
		break;

	default: {
		frame->eax = -EINVAL;
		printf("Unknown syscall %d!\n", num);
		break;
	}
	}

	return esp;
}

// Internal scheduler (=> yield) call
static u32 syscall_special_handler(u32 esp)
{
	if (proc_current())
		proc_reset_quantum(proc_current());
	return scheduler(esp);
}

CLEAR void syscall_init(void)
{
	int_special_handler_add(0, syscall_handler);
	int_special_handler_add(1, syscall_special_handler);
}
