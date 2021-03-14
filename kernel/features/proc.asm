%define USER_CODE_SEGMENT 0x18
%define USER_DATA_SEGMENT 0x20
%define RING3_MASK 0b11
%define INTERRUPT_FLAG 0x200

global proc_jump_userspace
extern _esp
extern _eip
proc_jump_userspace:
	cli

	mov ax, USER_DATA_SEGMENT | RING3_MASK
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov eax, dword [_esp]
	push USER_DATA_SEGMENT | RING3_MASK
	push eax
	pushf

	pop eax
	or eax, INTERRUPT_FLAG
	push eax

	push USER_CODE_SEGMENT | RING3_MASK
	push dword [_eip]

	iret

global paging_invalidate_tlb
paging_invalidate_tlb:
	mov eax, cr3
	mov cr3, eax
	ret
