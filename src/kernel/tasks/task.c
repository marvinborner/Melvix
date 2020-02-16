#include <kernel/memory/paging.h>
#include <kernel/tasks/task.h>
#include <kernel/memory/alloc.h>
#include <kernel/lib/lib.h>
#include <kernel/gdt/gdt.h>
#include <kernel/system.h>

task_t *current_task;
task_t *ready_queue;

extern uint32_t read_eip();

uint32_t next_pid = 1;

void tasking_install()
{
    asm ("cli");
    move_stack((void *) 0xE0000000, 0x2000);

    current_task = ready_queue = (task_t *) kmalloc(sizeof(task_t));
    current_task->id = (int) next_pid++;
    current_task->esp = 0;
    current_task->ebp = 0;
    current_task->eip = 0;
    current_task->page_directory = current_page_directory;
    current_task->next = 0;
    current_task->kernel_stack = kmalloc(KERNEL_STACK_SIZE);

    vga_log("Installed Tasking");
    asm ("sti");
}

void move_stack(void *new_stack_start, uint32_t size)
{
    for (uint32_t i = (uint32_t) new_stack_start;
         i >= ((uint32_t) new_stack_start - size);
         i -= 0x1000) {
        // paging_alloc_frame(paging_get_page(i, 1, current_page_directory), 0, 1);
    }

    uint32_t pd_addr;
    asm volatile ("mov %%cr3, %0" : "=r" (pd_addr));
    asm volatile ("mov %0, %%cr3" : : "r" (pd_addr));

    uint32_t old_stack_pointer; asm volatile ("mov %%esp, %0" : "=r" (old_stack_pointer));
    uint32_t old_base_pointer;  asm volatile ("mov %%ebp, %0" : "=r" (old_base_pointer));

    uint32_t offset = (uint32_t) new_stack_start - initial_esp;

    uint32_t new_stack_pointer = old_stack_pointer + offset;
    uint32_t new_base_pointer = old_base_pointer + offset;

    memcpy((void *) new_stack_pointer, (void *) old_stack_pointer, initial_esp - old_stack_pointer);

    for (uint32_t i = (uint32_t) new_stack_start; i > (uint32_t) new_stack_start - size; i -= 4) {
        uint32_t tmp = *(uint32_t *) i;
        if ((old_stack_pointer < tmp) && (tmp < initial_esp)) {
            tmp = tmp + offset;
            uint32_t *tmp2 = (uint32_t *) i;
            *tmp2 = tmp;
        }
    }

    asm volatile ("mov %0, %%esp" : : "r" (new_stack_pointer));
    asm volatile ("mov %0, %%ebp" : : "r" (new_base_pointer));
}

extern void perform_task_switch(uint32_t, uint32_t, uint32_t, uint32_t);

void switch_task()
{
    if (!current_task)
        return;

    uint32_t esp, ebp, eip;
    asm volatile ("mov %%esp, %0" : "=r"(esp));
    asm volatile ("mov %%ebp, %0" : "=r"(ebp));

    eip = read_eip();

    if (eip == 0x12345)
        return;

    current_task->eip = eip;
    current_task->esp = esp;
    current_task->ebp = ebp;

    current_task = current_task->next;
    if (!current_task) current_task = ready_queue;

    eip = current_task->eip;
    esp = current_task->esp;
    ebp = current_task->ebp;

    current_page_directory = current_task->page_directory;

    set_kernel_stack((uintptr_t) (current_task->kernel_stack + KERNEL_STACK_SIZE));

    paging_switch_directory((int) current_page_directory);
    perform_task_switch(eip, (uint32_t) current_page_directory, ebp, esp);
}

int fork()
{
    asm ("cli");

    task_t *parent_task = (task_t *) current_task;

    uint32_t *directory = 0;//paging_clone_directory(current_page_directory);

    task_t *new_task = (task_t *) kmalloc(sizeof(task_t));
    new_task->id = (int) next_pid++;
    new_task->esp = new_task->ebp = 0;
    new_task->eip = 0;
    new_task->page_directory = directory;
    current_task->kernel_stack = kmalloc(KERNEL_STACK_SIZE);
    new_task->next = 0;

    task_t *tmp_task = (task_t *) ready_queue;
    while (tmp_task->next)
        tmp_task = tmp_task->next;
    tmp_task->next = new_task;

    uint32_t eip = read_eip();

    if (current_task == parent_task) {
        uint32_t esp; asm volatile ("mov %%esp, %0" : "=r"(esp));
        uint32_t ebp; asm volatile ("mov %%ebp, %0" : "=r"(ebp));
        new_task->esp = esp;
        new_task->ebp = ebp;
        new_task->eip = eip;
        asm volatile ("sti");

        return new_task->id;
    } else {
        return 0;
    }
}

int getpid()
{
    return current_task->id;
}

void exec(uint32_t binary)
{
    set_kernel_stack((uintptr_t) (current_task->kernel_stack + KERNEL_STACK_SIZE));

    info("Switching to user mode...");

    asm volatile ("\
      cli; \
      mov $0x23, %%ax; \
      mov %%ax, %%ds; \
      mov %%ax, %%es; \
      mov %%ax, %%fs; \
      mov %%ax, %%gs; \
      mov %%esp, %%eax; \
      pushl $0x23; \
      pushl %%esp; \
      pushf; \
      pushl $0x1B; \
      push $1f; \
      iret; \
      1: \
      " : : "r" (binary));

    // syscall_write("test");
}