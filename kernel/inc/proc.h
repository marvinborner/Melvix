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

#define PROC_MAX_BLOCK_IDS 16
#define PROC_BLOCK_MAGIC 0x00528491

#define STREAM_MAX_SIZE 4096
enum stream_defaults { STREAM_IN, STREAM_OUT, STREAM_ERR, STREAM_LOG, STREAM_UNKNOWN = -1 };

enum proc_priv { PROC_PRIV_NONE, PROC_PRIV_ROOT, PROC_PRIV_KERNEL };
enum proc_state { PROC_RUNNING, PROC_BLOCKED };
enum proc_block_type { PROC_BLOCK_DEV, PROC_BLOCK_MSG };

struct proc_block_identifier {
	u32 magic;
	u32 id;
	enum proc_block_type type;
	u32 func_ptr;
};

struct proc_block {
	struct proc_block_identifier ids[PROC_MAX_BLOCK_IDS];
	u32 id_cnt;
};

struct stream {
	u32 offset_read;
	u32 offset_write;
	char data[STREAM_MAX_SIZE];
};

struct proc {
	u32 pid;
	u32 entry;
	u32 user_stack;
	u32 kernel_stack;

	char name[64];
	char dir[64];
	struct stream streams[4];
	struct page_dir *page_dir;
	struct regs regs;
	struct proc_block block; // dev_id
	enum proc_priv priv;
	enum proc_state state;
	struct stack *messages;
	struct list *memory;

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
void proc_yield(struct regs *r) NONNULL;
void proc_set_quantum(struct proc *proc, u32 value);
void proc_reset_quantum(struct proc *proc);
void proc_state(struct proc *proc, enum proc_state state);
void proc_block(u32 id, enum proc_block_type type, u32 func_ptr);
void proc_unblock(u32 id, enum proc_block_type type);
struct proc *proc_make(enum proc_priv priv);
void proc_stack_push(struct proc *proc, u32 data) NONNULL;

#endif
