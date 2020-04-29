section .multiboot
	header_start:
		dd 0xe85250d6
		dd 0
		dd header_end - header_start
		dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

		; Information tag
		align 8
		dw 1
		dw 1
		dd 24
		dd 2 ; bootloader name
		dd 4 ; meminfo
		dd 6 ; mmap
		dd 13 ; smbios

		; Empty tag
		align 8
		dw 0
		dw 0
		dd 8
	header_end:

section .start_section
	dd _start

; Initialize stack
;section .bss
;	align 16
;
;	STACK_BOTTOM:
;		resb 0x4000
;	STACK_TOP:

section .text
	global _start
	extern kernel_main
	_start:
		;mov esp, STACK_TOP
		push esp
		push ebx
		push eax
		cli
		call kernel_main
		; cli
		jmp $