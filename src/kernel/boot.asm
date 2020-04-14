%define ALIGN    1 ; Align loaded modules on page boundaries
%define MEMINFO  2 ; Provide memory map
%define FLAGS    3 ; Flags (ALIGN | MEMINFO)
%define MAGIC    0x1BADB002
%define CHECKSUM -(MAGIC + FLAGS)

section .multiboot
    align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .start_section
    dd _start

; Initialize stack
section .bss
    align 16
    global STACK_BOTTOM
    global STACK_TOP

    STACK_BOTTOM:
        resb 0x4000
    STACK_TOP:

section .text
    global _start
    extern kernel_main
    _start:
        mov esp, STACK_TOP
        push ebx
        push eax
        cli
        call kernel_main
        cli
        jmp $

    global jump_userspace
    jump_userspace:
        cli
        push ebp
        mov ebp, esp
        mov edx, dword [esp + 0xC]
        mov esp, edx
        push 0xABCDEF

        mov ax, 0x23
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax

        mov eax, esp
        push 0x23
        push eax
        pushf

        ; Enable interrupts
        pop eax
        or eax, 0x200
        push eax

        push 0x1B
        push dword [ebp + 8]
        iret

        pop ebp
        ret

section .end_section
    global ASM_KERNEL_END
    ASM_KERNEL_END:
        ; Kernel size detection