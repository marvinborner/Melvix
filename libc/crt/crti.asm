; MIT License, Copyright (c) 2020 Marvin Borner

section .init
global _init
_init:
	push ebp
	mov ebp, esp

section .fini
global _fini
fini:
	push ebp
	mov ebp, esp
