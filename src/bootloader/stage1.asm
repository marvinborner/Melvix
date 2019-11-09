org 0x7c00
bits 16

STAGE2 equ 0x800
STAGE2_SECTORS equ 2+1
TRACKS equ 2
mov [BOOT_DRIVE], dl

mov bp, 0x9000
mov sp, bp

start:
    jmp load_stage2

print:
    lodsb
    or al, al
    jz done
    mov ah, 0eh
    int 10h
    jmp print
done:
    ret

clear:
    pusha
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    popa
    ret

print_message:
    xor ax, ax
    mov ds, ax
    mov es, ax

    call clear
    mov si, msg
    call print

disk_load:
    pusha
    push dx
    mov ah, 0x02
    mov al, dh
    mov dh, 0x0
    int 0x13
    popa
    ret

load_stage2:
    call print_message
    mov cl, 2
    mov bx, STAGE2
    mov dh, 1
    mov dl, [BOOT_DRIVE]
load_sector:
    call disk_load
    cmp cl, STAGE2_SECTORS
    je loaded
    cmp cl, 15
    add cl, 1
    add bx, 512
    jmp load_sector
loaded:
    ret

msg db "Booting Melvix...", 0
BOOT_DRIVE db 0

times 510 - ($-$$) db 0
dw 0xAA55
