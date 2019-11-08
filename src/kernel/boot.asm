; The first section of the ELF will be used to locate the entry point.
section .ezlocation
dd _start

; Set stack
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

global switch_to_user
extern test_user
switch_to_user:
    sti
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
    push test_user
    iret

section .sizedetect
global ASM_KERNEL_END
ASM_KERNEL_END:
	; Kernel size detection
