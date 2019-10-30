[bits 32]
global start

start:
    mov esp, _sys_stack ; Points stack to stack area
    jmp stublet

; Align with 4 Bytes
ALIGN 4
mboot:
    ; Multiboot macros
    MULTIBOOT_PAGE_ALIGN equ 1<<0
    MULTIBOOT_MEMORY_INFO equ 1<<1
    MULTIBOOT_AOUT_KLUDGE equ 1<<16
    MULTIBOOT_HEADER_MAGIC equ 0x1BADB002
    MULTIBOOT_HEADER_FLAGS equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE
    MULTIBOOT_CHECKSUM	equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
    EXTERN code, bss, end

    ; GRUB Multiboot header
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_CHECKSUM

    ; AOUT kludge
    dd mboot
    dd code
    dd bss
    dd end
    dd start

; Endless loop
extern kernel_main
stublet:
    ; Load multiboot information
    push esp
    push ebx

    cli
    call kernel_main
    jmp $

%include "src/kernel/gdt/gdt.asm"

%include "src/kernel/interrupts/idt.asm"

%include "src/kernel/interrupts/isr.asm"

%include "src/kernel/interrupts/irq.asm"

%include "src/kernel/interact.asm"

global switch_to_user
switch_to_user:
     cli
     mov ax,0x23
     mov ds,ax
     mov es,ax
     mov fs,ax
     mov gs,ax

     mov eax,esp
     push 0x23
     push eax
     pushf
     pop eax
     or eax, 0x200
     push eax
     push 0x1B
     iret

; Store the stack
SECTION .bss
    resb 0x2000 ; Reserve 8KiB
_sys_stack: