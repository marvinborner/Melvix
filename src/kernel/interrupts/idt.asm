; IDT loader
global idt_load
extern idtp
idt_load:
    lidt [idtp]
    ret

global idt_syscall
extern syscall_handler
idt_syscall:
    push ds
    push es
    push fs
    push gs
    pushad

    push ecx
    push edx
    push esi
    push edi
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call syscall_handler

    lea ebx, [5 * 4]
    add esp, ebx

    mov dword [esp + (7*4)], eax

    popad
    pop gs
    pop fs
    pop es
    pop ds
    iret
