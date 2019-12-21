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

    hlt_L:
        hlt
        jmp hlt_L

    %include "src/kernel/gdt/gdt.asm"

    %include "src/kernel/interrupts/idt.asm"

    %include "src/kernel/interrupts/isr.asm"

    %include "src/kernel/interrupts/irq.asm"

    %include "src/kernel/interact.asm"

    global jump_userspace
    jump_userspace:
        push ebp
        mov ebp, esp
        mov edx, DWORD[ebp + 0xC]
        mov esp, edx

        mov ax, 0x23
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax

        mov eax, esp
        push 0x23
        push eax
        pushf
        pop eax

        or eax, 0x200
        push eax
        push 0x1B

        push DWORD[ebp + 0x8]

        iret
        pop ebp
        ret

section .end_section
    global ASM_KERNEL_END
    ASM_KERNEL_END:
        ; Kernel size detection
