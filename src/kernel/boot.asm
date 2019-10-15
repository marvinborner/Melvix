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
    MULTIBOOT_VIDEO_MODE equ 1<<2
    MULTIBOOT_HEADER_MAGIC equ 0x1BADB002
    MULTIBOOT_HEADER_FLAGS equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_VIDEO_MODE
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

    ; Request linear graphics mode
    dd 0
    dd 640 ; width
    dd 480 ; height
    dd 32 ; bpp

; Endless loop
stublet:
    extern kernel_main
    call kernel_main
    jmp $

[global copy_page_physical]
copy_page_physical:
    push ebx
    pushf
    cli
    mov ebx, [esp+12]
    mov ecx, [esp+16]

    ; Disable paging
    mov edx, cr0
    and edx, 0x7fffffff
    mov cr0, edx

    mov edx, 0x400

%include "src/kernel/gdt/gdt.asm"

%include "src/kernel/interrupts/idt.asm"

%include "src/kernel/interrupts/isr.asm"

%include "src/kernel/interrupts/irq.asm"

%include "src/kernel/interact.asm"

; Store the stack
SECTION .bss
    resb 0x2000 ; Reserve 8KiB
_sys_stack: