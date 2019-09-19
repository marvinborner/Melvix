global apm_check
global apm_connect
global apm_poweroff
global apm_sleep

extern apm_error

apm_check:
    mov ah,53h
    mov al,00h
    xor bx,bx
    int 15h
    jc apm_error
    ret

apm_connect:
    mov ah, 53h
    mov al, 03h
    xor bx, bx
    int 15h
    jc apm_error

apm_poweroff:
    mov ah, 53h
    mov al, 07h
    mov bx, 0001h
    mov cx, 03h
    int 15h
    jc apm_error

apm_sleep:
    mov ah, 53h
    mov al, 07h
    mov bx, 0001h
    mov cx, 01h
    int 15h
    jc apm_error