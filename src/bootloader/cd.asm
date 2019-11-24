BITS 16
ORG 0x7C00

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
    dapack_boffset: dw 0x9000
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

filenameLength dw 1
filename dw 1

findfile:
    mov bx, [esp+2]
    mov ax, [esp+4]
    mov [filenameLength], ax
    mov ax, [esp+6]
    mov [filename], ax

    .findfile_L:
        mov al, [bx]

        test al, al
        jz .findfile_notfound

        mov ah, [bx+32]
        cmp ah, byte [filenameLength]
        jnz .findfile_keep

        call findfile_check
        test ax, ax
        jz findfile_found

        .findfile_keep:
        mov al, [bx]
        xor ah, ah
        add bx, ax
        jmp .findfile_L

    .findfile_notfound:
        xor ax, ax
        inc ax
        ret

    findfile_check:
        pusha
        add bx, 33
        mov ax, bx

        xor cx, cx
        .findfile_check_L:
            mov bx, [filename]
            add bx, cx
            mov dh, [bx]
            mov bx, ax
            add bx, cx
            mov dl, [bx]
            cmp dh, dl
            jnz .findfile_check_fail

            inc cx
            cmp cx, word [filenameLength]
            jz .findfile_check_success
            jmp .findfile_check_L

            .findfile_check_fail:
                popa
                xor ax, ax
                inc ax
                ret
            .findfile_check_success:
                popa
                xor ax, ax
                ret

    findfile_found:
        xor ax, ax
        ret

LOAD_PVD:
    mov eax, 0x10
    PVD_L:
        call readsector
        mov bx, [dapack_boffset]
        mov bl, [bx]
        cmp bl, 0x01
        jz PVD_FOUND
        inc eax
        jmp PVD_L
    PVD_FOUND:
    ret

checkA20:
    ; Stolen from https://wiki.osdev.org/A20_Line
    pushf
    push ds
    push es
    push di
    push si
    cli
    xor ax, ax
    mov es, ax
    not ax
    mov ds, ax
    mov di, 0x0500
    mov si, 0x0510
    mov al, byte [es:di]
    push ax
    mov al, byte [ds:si]
    push ax
    mov byte [es:di], 0x00
    mov byte [ds:si], 0xFF
    cmp byte [es:di], 0xFF
    pop ax
    mov byte [ds:si], al
    pop ax
    mov byte [es:di], al
    mov ax, 0
    jz checkA20_exit
    mov ax, 1
    checkA20_exit:
        pop si
        pop di
        pop es
        pop ds
        popf
        ret

welcome db "Melvix", 0x0A, 0x0D, 0x00
nolba db "BIOS lacks support for lba addressing.", 0x00
noboot db "Boot directory could not be found.", 0x00
noa20 db "A20 could not be enabled.", 0
loading db "Loading kernel...", 0x0A, 0x0D, 0x00
nokernel db "kernel.bin could not be found!", 0
booting db "Booting...", 0x0A, 0x0D, 0x00
nomem db "BIOS does not support memory detection!", 0
memno20 db "BIOS returns memory detection with 24 bytes. This has never been seen!", 0

start:
    ; Check if lba is supported by the BIOS.
    mov ah, 0x41
    mov bx, 0x55AA
    int 0x13
    jc lba_not_supported
    cmp bx, 0xAA55
    jnz lba_not_supported

    call checkA20
    test ax, ax
    jnz A20_ENABLED

    in al, 0x92
    or al, 2
    out 0x92, al

    call checkA20
    test ax, ax
    jnz A20_ENABLED

    mov si, noa20
    call print
    jmp $

A20_ENABLED:
    ; Inspired by https://wiki.osdev.org/Unreal_Mode
    cli
    push ds
    lgdt [gdtinfo]

    mov eax, cr0
    or al, 1
    mov cr0, eax

    jmp $+2

    mov bx, 0x08
    mov ds, bx

    and al, 0xFE
    mov cr0, eax
    pop ds

    mov si, loading
    call print

    call LOAD_PVD

    mov bx, 0x9000
    add bx, 156
    add bx, 2
    mov eax, dword [bx]
    call readsector

    push boot
    push boot_len
    push 0x9000
    call findfile
    add esp, 6
    test ax, ax
    jz continue_BOOT

    mov si, noboot
    call print
    jmp $

continue_BOOT:
    mov word [dapack_blkcount], 0x0001
    add bx, 2
    mov eax, [bx]
    call readsector

    push kernelbin
    push kernelbin_len
    push 0x9000
    call findfile
    add esp, 6
    test ax, ax
    jz continue_KERNEL

    mov si, nokernel
    call print
    jmp $

continue_KERNEL:
    mov word [dapack_blkcount], 1
    mov word [dapack_boffset], 0x9000

    push bx
    add bx, 10
    mov eax, [bx]
    xor edx, edx
    mov ebx, 2048
    div ebx
    inc eax
    pop bx
    push eax

    add bx, 2
    mov eax, [bx]
    push eax

    ; Offsetless count.
    xor ecx, ecx
LOAD_KERNEL:
    mov eax, [esp]
    add eax, ecx
    call readsector

    mov eax, ecx
    shl eax, 11
    add eax, 0x200000

    push ecx
    xor ecx, ecx
    .LOAD_KERNEL_L:
        mov ebx, ecx
        add ebx, 0x9000
        mov ebx, dword [ebx]

        mov edx, ebx
        mov ebx, eax
        add ebx, ecx
        mov dword [ebx], edx

        add ecx, 4
        cmp ecx, 2048
        jl .LOAD_KERNEL_L
    pop ecx

    inc ecx
    cmp ecx, dword [esp+4]
    jl LOAD_KERNEL

mov si, booting
call print

mov dl, [bootDriveID]
mov byte [0x9000], dl

mov di, 0xA000
mov eax, 0xE820
xor ebx, ebx
mov ecx, 24
mov edx, 0x534D4150
int 0x15

jc BIOS_NO_MEM
cmp eax, 0x534D4150
jnz BIOS_NO_MEM
cmp cl, 20
jnz BIOS_MEM_NO20

MEM_L:
    test ebx, ebx
    jz MEM_FINISHED

    mov ax, di
    xor ch, ch
    add ax, cx
    mov di, ax

    mov eax, 0xE820
    mov ecx, 24
    int 0x15
    jmp MEM_L

MEM_FINISHED:
    mov eax, cr0
    or al, 1
    mov cr0, eax

    mov ax, 0x08
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

; Infomation: https://wiki.osdev.org/ELF
mov ebx, 0x20002C
mov dl, byte [ebx]
push dx

mov ebx, 0x20001C
mov ebx, dword [ebx]
push ebx

xor dh, dh
PHT:
    xor eax, eax
    mov al, dh
    shl eax, 5
    add eax, dword [esp]
    add eax, 0x200000
    mov ebx, eax

    mov eax, dword [ebx]
    cmp eax, 1
    jnz .PHT_ignore

    add ebx, 4
    mov eax, dword [ebx]
    add eax, 0x200000
    push eax

    add ebx, 4
    mov eax, dword [ebx]
    push eax

    add ebx, 8
    mov eax, dword [ebx]
    push eax

    push dx
    xor ecx, ecx
    .MOVE_KERNEL_L:
        mov ebx, [esp+10]
        add ebx, ecx
        mov ebx, dword [ebx]

        mov eax, ebx
        mov ebx, dword [esp+6]
        add ebx, ecx
        mov dword [ebx], eax

        add ecx, 4
        cmp ecx, dword [esp+2]
        jl .MOVE_KERNEL_L
    pop dx
    add esp, 12

    .PHT_ignore:
        inc dh
        cmp dh, dl
        jl PHT

jmp (codedesc - gdt):protectedMode

protectedMode:
    BITS 32
    mov ebx, 0x100000
    mov eax, dword [ebx]
    jmp eax

BITS 16
BIOS_NO_MEM:
    mov si, nomem
    call print
    jmp $
BIOS_MEM_NO20:
    mov si, memno20
    call print
    jmp $

gdtinfo:
    dw gdt_end - gdt - 1
    dd gdt
    gdt dd 0, 0
    flatdesc db 0xff, 0xff, 0, 0, 0, 10010010b, 11001111b, 0
    codedesc db 0xff, 0xff, 0, 0, 0, 10011010b, 11001111b, 0
    gdt_end:

lba_not_supported:
    mov si, nolba
    call print
    jmp $

boot db "BOOT"
boot_len equ ($ - boot)
kernelbin db "KERNEL.BIN", 0x3B, "1"
kernelbin_len equ ($ - kernelbin)
