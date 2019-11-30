bits 32
mov esp, ebp

mov eax, 1
lea edi, [ebp+welcome]
mov esi, welcome_sz
int 0x80

jmp $

welcome db "Welcome to the userspace!", 0x0A, 0x0A
welcome_sz equ $ - welcome