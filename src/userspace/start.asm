bits 32
section .start_section
    dd _start

section .text
    global _start
    extern user_main
    _start:
        mov esp, ebp
        call user_main

    global syscall
    syscall:
        mov eax, 1
        lea edi, [ebp+welcome]
        mov esi, welcome_sz
        int 0x80
        ret

section .data
        welcome db "Welcome to the userspace", 0x0A, 0x0A
        welcome_sz equ $ - welcome