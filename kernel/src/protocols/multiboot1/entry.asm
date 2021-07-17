; MIT License, Copyright(c) 2021 Marvin Borner

bits 32

%define MULTIBOOT_MAGIC 0x1badb002
%define MULTIBOOT_PAGE_ALIGN 0x1
%define MULTIBOOT_MEMORY_INFO 0x2
%define MULTIBOOT_VIDEO_MODE 0x4
%define MULTIBOOT_FLAGS (MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_VIDEO_MODE)
%define MULTIBOOT_CHECKSUM -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .text
align 4

dd MULTIBOOT_MAGIC
dd MULTIBOOT_FLAGS
dd MULTIBOOT_CHECKSUM

; MULTIBOOT_MEMORY_INFO
dd 0x00000000
dd 0x00000000
dd 0x00000000
dd 0x00000000
dd 0x00000000

; MULTIBOOT_VIDEO_MODE
dd 0x00000000
dd 1920
dd 1200
dd 32

global boot_entry
extern arch_init
boot_entry:
	mov esp, stack_top
	push ebx
	push eax
	cli
	call arch_init
	jmp $

section .bss
align 32
stack_bottom:
	resb 0x4000
stack_top:
