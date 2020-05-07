#ifndef MELVIX_USERSPACE_H
#define MELVIX_USERSPACE_H

#include <interrupts/interrupts.h>
#include <stdint.h>
#include <tasks/process.h>

u32 spawn_child(struct process *child);

void userspace_enter(struct process *proc);

void single_yield(struct process *proc, struct regs *regs);
u32 single_exit(struct regs *regs);

extern void jump_userspace();

#endif