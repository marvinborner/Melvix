// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef PROC_H
#define PROC_H

#include <def.h>
#include <interrupts.h>
#include <list.h>
#include <stack.h>
#include <sys.h>

#define PROC_QUANTUM 10 // Milliseconds or something // TODO

#define EFLAGS_ALWAYS 0x2 // Always one
#define EFLAGS_INTERRUPTS 0x200 // Enable interrupts

#define GDT_USER_CODE_OFFSET 0x1b // User code segment offset in GDT (with ring3 mask)
#define GDT_USER_DATA_OFFSET 0x23 // User data segment offset in GDT (with ring3 mask)

#define PROC_MAX_WAIT_IDS 16
#define PROC_WAIT_MAGIC 0x00528491

#define STREAM_MAX_SIZE 4096
enum stream_defaults { STREAM_IN, STREAM_OUT, STREAM_ERR, STREAM_LOG, STREAM_UNKNOWN = -1 };

enum proc_state { PROC_RUNNING, PROC_SLEEPING };
enum proc_wait_type { PROC_WAIT_DEV, PROC_WAIT_MSG };

struct proc_wait_identifier {
	u32 magic;
	u32 id;
	enum proc_wait_type type;
	u32 func_ptr;
};

struct proc_wait {
	struct proc_wait_identifier ids[PROC_MAX_WAIT_IDS];
	u32 id_cnt;
};

struct stream {
	u32 offset_read;
	u32 offset_write;
	char data[STREAM_MAX_SIZE];
};

struct proc {
	u32 pid;
	u8 super;
	char name[32];
	struct stream streams[4];
	struct regs regs;
	struct proc_wait wait; // dev_id
	enum proc_state state;
	struct stack *messages;
};

void scheduler(struct regs *regs);
void proc_init(void);
void proc_print(void);
struct proc *proc_current(void);
u8 proc_super(void);
struct proc *proc_from_pid(u32 pid);
void proc_exit(struct proc *proc, int status);
void proc_yield(struct regs *r);
void proc_clear_quantum(void);
void proc_enable_waiting(u32 id, enum proc_wait_type type);
void proc_wait_for(u32 id, enum proc_wait_type type, u32 func_ptr);
struct proc *proc_make(void);

#endif
