; MIT License, Copyright (c) 2021 Marvin Borner

%macro INTERRUPT_REGISTER 1
dd int%1
%endmacro

%macro INTERRUPT_DEFINE 1
int%1:
	push 0
	push %1
	jmp interrupt_common
%endmacro

extern interrupt_handler
interrupt_common:
	cld

	pushad
	push ds
	push es
	push fs
	push gs

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	push esp
	call interrupt_handler
	mov esp, eax

	pop gs
	pop fs
	pop es
	pop ds
	popad

	add esp, 8
	iret

%assign i 0
%rep 255
INTERRUPT_DEFINE i
%assign i i+1
%endrep

global interrupt_table
interrupt_table:
	%assign i 0
	%rep 255
	INTERRUPT_REGISTER i
	%assign i i+1
	%endrep
