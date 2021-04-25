// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef PROC_H
#define PROC_H

#include <def.h>
#include <interrupts.h>
#include <list.h>
#include <stack.h>
#include <sys.h>

#define PROC_QUANTUM 42 // Milliseconds or something // TODO
#define PROC_STACK_SIZE (1 << 20) // 1MiB

#define EFLAGS_ALWAYS 0x2 // Always one
#define EFLAGS_INTERRUPTS 0x200 // Enable interrupts

#define GDT_SUPER_CODE_OFFSET 0x08 // Super (kernel) code segment offset in GDT
#define GDT_SUPER_DATA_OFFSET 0x10 // Super (kernel) data segment offset in GDT
#define GDT_USER_CODE_OFFSET 0x1b // User code segment offset in GDT (with ring3 mask)
#define GDT_USER_DATA_OFFSET 0x23 // User data segment offset in GDT (with ring3 mask)

#define RING(regs) ((regs->cs) & 3)

enum proc_priv { PROC_PRIV_NONE, PROC_PRIV_ROOT, PROC_PRIV_KERNEL };
enum proc_state { PROC_RUNNING, PROC_BLOCKED };

struct proc {
	u32 pid;
	u32 entry;

	char name[64];
	char dir[64];
	struct page_dir *page_dir;
	struct regs regs;
	enum proc_priv priv;
	enum proc_state state;
	struct stack *messages;
	struct list *memory;

	u32 bus_conn;

	struct {
		u32 user;
		u32 kernel;
	} stack;

	struct {
		u32 user;
		u32 kernel;
	} ticks;

	struct {
		u8 val;
		u8 cnt;
	} quantum;
};

void scheduler(struct regs *regs) NONNULL;
void proc_init(void);
void proc_print(void);
struct proc *proc_current(void);
u8 proc_super(void);
struct proc *proc_from_pid(u32 pid);
void proc_exit(struct proc *proc, struct regs *r, s32 status) NONNULL;
void proc_yield(void);
void proc_set_quantum(struct proc *proc, u32 value);
void proc_reset_quantum(struct proc *proc);
void proc_state(struct proc *proc, enum proc_state state);
struct proc *proc_make(enum proc_priv priv);
void proc_stack_push(struct proc *proc, u32 data) NONNULL;

#endif
