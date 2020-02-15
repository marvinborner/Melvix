#ifndef MELVIX_TASK_H
#define MELVIX_TASK_H

#include <stdint.h>
#include <kernel/memory/paging.h>

#define KERNEL_STACK_SIZE 2048

typedef struct task {
    int id;
    uint32_t esp, ebp;
    uint32_t eip;
    page_directory_t *page_directory;
    uint32_t kernel_stack;
    struct task *next;
} task_t;

void tasking_install();

void switch_task();

int fork();

void move_stack(void *new_stack_start, uint32_t size);

int getpid();

void exec(uint32_t binary);

#endif
