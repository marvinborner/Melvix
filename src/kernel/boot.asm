bits 32

kernel_stack equ 0x4000
multiboot_magic equ 0xe85250d6

section .text
	align 4

	multiboot:
	header_start:
		dd multiboot_magic
		dd 0
		dd header_end - header_start
		dd 0x100000000 - (multiboot_magic + 0 + (header_end - header_start))

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

	global boot
	extern kernel_main
	boot:
		mov esp, stack_top
		push esp
		push ebx
		push eax
		cli
		call kernel_main
		hlt
		jmp $

section .bss
	align 32
	stack_bottom:
		resb kernel_stack
	stack_top: