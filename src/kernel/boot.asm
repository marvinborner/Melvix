bits 32

; Some variables
kernel_virt_base equ 0xC0000000
kernel_page_number equ (kernel_virt_base >> 22)
kernel_stack equ 0x4000

section .data
	align 0x1000

	boot_page_dir:
		dd 0x00000083
		times (kernel_page_number - 1) dd 0
		dd 0x00000083
		times (1024 - kernel_page_number - 1) dd 0

section .text
	align 4

	multiboot:
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

		; Page-align tag
		align 8
		dw 6
		dw 0
		dd 8

		; Empty tag
		align 8
		dw 0
		dw 0
		dd 8
	header_end:

	global boot
	extern kernel_main
	boot:
		mov ecx, (boot_page_dir - kernel_virt_base)
		mov cr3, ecx

		mov ecx, cr4
		or ecx, 0x00000010
		mov cr4, ecx
		
		mov ecx, cr0
		or ecx, 0x80000000
		mov cr0, ecx

		lea ecx, [higher_half]
		jmp ecx

	higher_half:
		mov dword [boot_page_dir], 0
		invlpg [0]
		
		mov esp, stack + kernel_stack
		push esp
		push ebx
		push eax
		cli
		call kernel_main
		hlt

section .bss
	align 32
	stack:
		resb kernel_stack