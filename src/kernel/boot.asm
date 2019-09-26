global start ; Make start available to linker
extern kernel_main ; Get main function from kernel.c

; Set Multiboot headers for GRUB
MODULEALIGN equ  1<<0
MEMINFO     equ  1<<1
FLAGS       equ  MODULEALIGN | MEMINFO
MAGIC       equ  0x1BADB002
CHECKSUM    equ  -(MAGIC + FLAGS)

; Set virtual base address of kernel space
KERNEL_VIRTUAL_BASE equ 0xC0000000 ; 3GB
KERNEL_PAGE_NUMBER equ (KERNEL_VIRTUAL_BASE >> 22) ; Page directory index of kernel's 4MB PTE.

section .data
align 0x1000
BootPageDirectory:
    ; Create page directory entry to identity-map the first 4MB of the 32-bit physical address space.
    dd 0x00000083
    times (KERNEL_PAGE_NUMBER - 1) dd 0
    ; Create 4MB page directory entry to contain the kernel
    dd 0x00000083
    times (1024 - KERNEL_PAGE_NUMBER - 1) dd 0

section .text
align 4
MultiBootHeader:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; Reserve 16k for initial kernel stack space
STACKSIZE equ 0x4000

; Set up entry point for linker
loader equ (start - 0xC0000000)
global loader

start:
    ; Using physical addresses for now
    mov ecx, (BootPageDirectory - KERNEL_VIRTUAL_BASE)
    mov cr3, ecx ; Load Page Directory Base Register

    mov ecx, cr4
    or ecx, 0x00000010 ; Set PSE in CR4 to enable 4MB pages
    mov cr4, ecx

    mov ecx, cr0
    or ecx, 0x80000000 ; Enable paging
    mov cr0, ecx

    ; Long jump to StartInHigherHalf
    lea ecx, [StartInHigherHalf]
    jmp ecx

StartInHigherHalf:
    mov dword [BootPageDirectory], 0
    invlpg [0]

    mov esp, stack+STACKSIZE           ; Set up the stack
    push eax                           ; Pass Multiboot magic number

    ; Pass Multiboot info structure
    push ebx
    add ebx, KERNEL_VIRTUAL_BASE
    push ebx

    call kernel_main
    hlt

%include "src/kernel/gdt/gdt.asm"

%include "src/kernel/interrupts/idt.asm"

%include "src/kernel/interrupts/isr.asm"

%include "src/kernel/interrupts/irq.asm"

%include "src/kernel/interact.asm"

section .bss
align 32
stack:
    resb STACKSIZE      ; reserve 16k stack on a uint64_t boundary
