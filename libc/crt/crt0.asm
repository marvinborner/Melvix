; MIT License, Copyright (c) 2020 Marvin Borner

section .text

extern main
extern sys1

global _start
_start:
	call main

	push eax
	push 9
	call sys1
	jmp $
