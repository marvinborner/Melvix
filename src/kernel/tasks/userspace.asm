global userspace_jump
extern hl_esp
extern hl_eip

userspace_jump:
	mov ax, 0x23
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov eax, dword [hl_esp]
	push 0x23
	push eax
	pushf

	; Enable interrupts
	sti

	push 0x1B
	push dword [hl_eip]

	iret