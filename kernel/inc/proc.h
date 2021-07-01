// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef PROC_H
#define PROC_H

#include <def.h>
#include <drivers/int.h>
#include <list.h>
#include <stack.h>
#include <sys.h>
#include <timer.h>

#define PROC_QUANTUM 15 // Milliseconds or something // TODO
#define PROC_STACK_SIZE 0x4000 // 16KiB

#define EFLAGS_ALWAYS 0x2 // Always one
#define EFLAGS_INTERRUPTS 0x200 // Enable interrupts

#define RING(regs) ((regs->cs) & 3)

enum proc_priv { PROC_PRIV_NONE, PROC_PRIV_ROOT, PROC_PRIV_KERNEL };
enum proc_state { PROC_RUNNING, PROC_BLOCKED };

struct proc {
	u32 pid;
	u32 entry;

	char name[64];
	char dir[64];
	u8 fpu[512];
	struct page_dir *page_dir;
	enum proc_priv priv;
	enum proc_state state;
	struct stack *messages;
	struct list *memory;

	u32 bus_conn;

	struct {
		u32 user;
		u32 user_ptr;
		u32 kernel;
		u32 kernel_ptr;
	} stack;

	struct {
		u32 user;
		u32 kernel;
	} ticks;

	struct {
		u8 val;
		u8 cnt;
	} quantum;

	struct {
		enum timer_mode mode;
		u32 data;
	} timer;
};

u32 scheduler(u32 esp);
NORETURN void proc_init(void);
void proc_print(void);
struct proc *proc_current(void);
u8 proc_super(void);
u8 proc_idle(void);
struct proc *proc_from_pid(u32 pid);
void proc_exit(s32 status);
void proc_yield(void);
void proc_timer_check(u32 time);
void proc_set_quantum(struct proc *proc, u32 value) NONNULL;
void proc_reset_quantum(struct proc *proc) NONNULL;
void proc_state(struct proc *proc, enum proc_state state) NONNULL;
struct proc *proc_make(enum proc_priv priv);
void proc_make_regs(struct proc *proc) NONNULL;
void proc_stack_user_push(struct proc *proc, const void *data, u32 size) NONNULL;
void proc_stack_kernel_push(struct proc *proc, const void *data, u32 size) NONNULL;

#endif
