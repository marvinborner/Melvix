bits 32

section .text
    extern user_main
    _start:
        mov esp, ebp
        call user_main
        jmp $