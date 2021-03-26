; MIT License, Copyright (c) 2020 Marvin Borner

section .text

extern main
extern exit
extern atexit_trigger

global _start
_start:
	call main

	push eax
	call exit
	jmp $
