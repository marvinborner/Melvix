#ifndef MELVIX_USERSPACE_H
#define MELVIX_USERSPACE_H

#include <stdint.h>
#include <kernel/tasks/process.h>
#include <kernel/interrupts/interrupts.h>

uint32_t spawn_child(struct process *child);

void userspace_enter(struct process *proc);

void single_yield(struct process *proc, struct regs *regs);
uint32_t single_exit(struct regs *regs);

extern void jump_userspace();

#endif