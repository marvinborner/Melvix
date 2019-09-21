[BITS 32]
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
stublet:
    extern kernel_main
    call kernel_main
    jmp $

%include "src/kernel/gdt/gdt.asm"

%include "src/kernel/interrupts/idt.asm"

%include "src/kernel/interrupts/isr.asm"

%include "src/kernel/interrupts/irq.asm"

; %include "src/kernel/graphics/vesa.asm"

; Store the stack
SECTION .bss
    resb 8192 ; Reserve 8KiB
_sys_stack: