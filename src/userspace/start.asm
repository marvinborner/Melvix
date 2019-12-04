bits 32
section .start_section
    dd _start

section .text
    global _start
    extern user_main
    _start:
        mov esp, ebp
        call user_main
        jmp $