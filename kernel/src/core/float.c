// MIT License, Copyright (c) 2020 Marvin Borner

#include <mem.h>
#include <print.h>

#include <core/features.h>
#include <core/float.h>

PROTECTED static u8 fpu_initial[512] ALIGNED(16);
static u8 fpu_regs[512] ALIGNED(16);

static void fpu_handler(void)
{
	__asm__ volatile("clts");
}

TEMPORARY void fpu_enable(void)
{
	// Enable FPU
	if (cpu_features.edx & CPUID_FEAT_EDX_FPU) {
		__asm__ volatile("fninit");
		__asm__ volatile("fxsave %0" : "=m"(fpu_initial));
		/* int_event_handler_add(7, fpu_handler); */
	} else {
		panic("No FPU support!\n");
	}
}

void fpu_init(struct proc *proc)
{
	memcpy(&proc->fpu, &fpu_initial, sizeof(fpu_initial));
}

void fpu_save(struct proc *proc)
{
	__asm__ volatile("fxsave (%0)" ::"r"(fpu_regs));
	memcpy(&proc->fpu, &fpu_regs, sizeof(fpu_regs));
}

void fpu_restore(struct proc *proc)
{
	memcpy(&fpu_regs, &proc->fpu, sizeof(proc->fpu));
	__asm__ volatile("fxrstor (%0)" ::"r"(fpu_regs));
}
