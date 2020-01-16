bits 32

section .text
    global _start
    extern user_main
    _start:
        call user_main