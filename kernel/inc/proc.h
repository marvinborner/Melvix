// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef PROC_H
#define PROC_H

#include <def.h>
#include <interrupts.h>
#include <list.h>
#include <stack.h>
#include <sys.h>

#define PROC_QUANTUM 42 // Milliseconds or something // TODO

#define EFLAGS_ALWAYS 0x2 // Always one
#define EFLAGS_INTERRUPTS 0x200 // Enable interrupts

#define GDT_USER_CODE_OFFSET 0x1b // User code segment offset in GDT (with ring3 mask)
#define GDT_USER_DATA_OFFSET 0x23 // User data segment offset in GDT (with ring3 mask)

#define PROC_MAX_WAIT_IDS 16

#define STREAM_IN 0
#define STREAM_OUT 1
#define STREAM_ERR 2
#define STREAM_LOG 3

enum proc_state { PROC_RUNNING, PROC_SLEEPING };
enum proc_wait_type { PROC_WAIT_DEV };

struct proc_wait {
	enum proc_wait_type type;
	u32 ids[PROC_MAX_WAIT_IDS]; // dev_id
	u32 id_cnt;
	s32 (*func)();
};

struct stream {
	u32 offset;
	u32 pos;
	char data[4096];
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
void proc_clear_quantum();
void proc_enable_waiting(u32 id, enum proc_wait_type type);
void proc_wait_for(u32 id, enum proc_wait_type type, s32 (*func)());
struct proc *proc_make(void);

#endif
