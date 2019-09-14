// Constants for the multiboot header
.set ALIGN, 1<<0
.set MEMINFO, 1<<1
.set FLAGS, ALIGN | MEMINFO
.set MAGIC, 0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

// Header marking the program as kernel
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

// Initialize a small stack
.section .bss
.align 16
stack_bottom:
.skip 16384 // 16 KiB
stack_top:

// Use _start from linker as starting point
.section .text
.global _start
.type _start, @function
_start:
	// Set up stack by setting esp to top of stack
	mov $stack_top, %esp

	// TODO: Initialize processor, load GDT, enable paging

	// Call the kernel
	call kernel_main

	// Put the system in an infinite loop
	cli
1:	hlt
	jmp 1b

// Set the size of the _start symbol to the current location '.' minus its start
.size _start, . - _start
