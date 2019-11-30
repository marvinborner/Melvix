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
        mov ebx, dword [esp+4]

        mov ax, 0x23
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax

        mov eax, esp
        push 0x23
        push eax
        pushf

        push 0x1B
        push ebx
        mov ebp, ebx
        iret

section .end_section
    global ASM_KERNEL_END
    ASM_KERNEL_END:
        ; Kernel size detection
