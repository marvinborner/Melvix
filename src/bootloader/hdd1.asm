BITS 16
ORG 0x7C00

%define INODE_ADDRESS 0xF000
%define INODE_NBLOCKS 20
%define INODE_DBP0 24

; Save the boot drive id
mov [bootDriveID], dl

jmp start

print:
    mov ah, 0x0E
    xor bh, bh
    .print_L:
        lodsb
        test al, al
        jz .print_end
        int 0x10
        jmp .print_L
    .print_end:
    ret

bootDriveID: db 0

dapack:
    dapack_size: db 0x10
    dapack_null: db 0x00
    dapack_blkcount: dw 0x0001
    dapack_boffset: dw 0x8000
    dapack_bsegment: dw 0x0000
    dapack_start: dd 0x00000000
    dapack_upper_lba_bits: dd 0x00000000

readsector:
    mov dword [dapack_start], eax
    mov ah, 0x42
    mov dl, [bootDriveID]
    xor bx, bx
    mov ds, bx
    mov si, dapack
    int 0x13
    ret

; 5K max
readwholefile:
    mov bx, word [dapack_boffset]
    push bx
    mov bx, INODE_ADDRESS
    mov word [dapack_boffset], bx
    call readsector
    pop bx
    mov word [dapack_boffset], bx

    mov edx, [INODE_ADDRESS+INODE_NBLOCKS]

    xor ecx, ecx
    .readwholefile_L:
        lea bx, [INODE_ADDRESS + INODE_DBP0 + (ecx*4)]
        mov eax, dword [bx]

        push dx
        call readsector
        pop dx

        add dword [dapack_boffset], 512

        inc cx
        cmp cx, dx
        jl .readwholefile_L
    ret

welcome db "Melvix", 0x0A, 0x0D, 0x00
nolba db "BIOS lacks support for LBA addressing.", 0x00
signaturebad db "Bad disk signature.", 0x00

start:
    mov ax, 0x0003
    int 0x10

    mov si, welcome
    call print

    mov ah, 0x41
    mov bx, 0x55AA
    int 0x13
    jc lba_not_supported
    cmp bx, 0xAA55
    jnz lba_not_supported

    mov eax, 1
    call readsector

    mov eax, [0x8000+0]
    cmp eax, 0x34B59645
    jnz signature_BAD
    mov eax, [0x8000+4]
    cmp eax, 0x1083B99F
    jnz signature_BAD

    mov eax, 2
    call readwholefile

    jmp 0x8000

lba_not_supported:
    mov si, nolba
    call print
    jmp $

signature_BAD:
    mov si, signaturebad
    call print
    jmp $

times 510-($-$$) db 0
dw 0xAA55