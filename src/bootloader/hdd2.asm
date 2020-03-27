BITS 16
ORG 0x8000

%define SECTOR_BUFFER 0x9000
%define INODE_NBLOCKS 20
%define INODE_DBP0 24
%define INODE_SIBP 64
%define INODE_DIBP 68
%define INODE_TIBP 72
%define INODE_QIBP 76

mov esp, 0xFFFF
jmp start

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

last_max_level db 0
getrecursive:
    push edi
    push esi
    push ecx

    mov eax, edi
    cmp al, byte [last_max_level]
    jna .last_max_level_updated
    mov byte [last_max_level], al
    .last_max_level_updated:

    mov eax, esi
    call 0x7c26

    mov eax, dword [esp]
    sub eax, 10

    cmp byte [last_max_level], 1
    jna .check1
    mov ebx, 1
    shl ebx, 7
    sub eax, ebx
    .check1:
        cmp byte [last_max_level], 2
        jna .check2
        mov ebx, 1
        shl ebx, 14
        sub eax, ebx
    .check2:
        cmp byte [last_max_level], 3
        jna .check3
        mov ebx, 1
        shl ebx, 21
        sub eax, ebx
    .check3:
        mov ecx, dword [esp+8]
        dec ecx
        push ecx
        shl ecx, 3
        sub ecx, dword [esp]
        add esp, 4
        shr eax, cl

        mov eax, dword [SECTOR_BUFFER+(4*eax)]

        cmp dword [esp+8], 1
        jna .finish_recursion
        mov edi, [esp+8]
        dec edi
        mov esi, eax
        mov ecx, [esp]
        call getrecursive
    .finish_recursion:
        mov byte [last_max_level], 0

        pop ecx
        pop esi
        pop edi
        ret

get_block:
    push ecx
    push ebx

    xor ebx, ebx
    inc ebx
    shl ebx, 21
    add ebx, 9
    cmp ecx, ebx
    jna .gtb1

    mov edi, 4
    mov esi, dword [SECTOR_BUFFER+INODE_QIBP]
    call getrecursive
    jmp .get_block_end

    .gtb1:
        xor ebx, ebx
        inc ebx
        shl ebx, 14
        add ebx, 9
        cmp ecx, ebx
        jna .gtb2

        mov edi, 3
        mov esi, dword [SECTOR_BUFFER+INODE_TIBP]
        call getrecursive
        jmp .get_block_end

    .gtb2:
        xor ebx, ebx
        inc ebx
        shl ebx, 7
        add ebx, 9
        cmp ecx, ebx
        jna .gtb3

        mov edi, 2
        mov esi, dword [SECTOR_BUFFER+INODE_DIBP]
        call getrecursive
        jmp .get_block_end

    .gtb3:
        cmp ecx, 9
        jna .gtb4

        mov edi, 1
        mov esi, dword [SECTOR_BUFFER+INODE_SIBP]
        call getrecursive
        jmp .get_block_end

    .gtb4:
        mov eax, SECTOR_BUFFER+INODE_DBP0
        mov eax, dword [eax+(4*ecx)]

    .get_block_end:
        pop ebx
        pop ecx
        ret

noa20 db "A20 could not be enabled.", 0
loading db "Loading kernel...", 0x0A, 0x0D, 0x00
booting db "Booting...", 0x0A, 0x0D, 0x00
nomem db "BIOS does not support memory detection!", 0
memno20 db "BIOS returns memory detection with 24 bytes. This has never been seen!", 0

start:
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
    call 0x7c07
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
    call 0x7c07

    mov word [0x7c1a], SECTOR_BUFFER

    mov eax, 3
    call 0x7c26
    mov edx, dword [SECTOR_BUFFER + INODE_NBLOCKS]

    xor ecx, ecx
LOAD_KERNEL:
    push edx

    mov eax, 3
    call 0x7c26
    call get_block
    call 0x7c26

    mov eax, ecx
    shl eax, 9
    add eax, 0x200000

    push ecx
    xor ecx, ecx
    .LOAD_KERNEL_L:
        mov ebx, ecx
        add ebx, SECTOR_BUFFER
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
    pop edx
    cmp ecx, edx
    jl LOAD_KERNEL

mov si, booting
call 0x7c07

mov dl, [0x7c15]
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
    call 0x7c07
    jmp $
BIOS_MEM_NO20:
    mov si, memno20
    call 0x7c07
    jmp $

gdtinfo:
    dw gdt_end - gdt - 1
    dd gdt
    gdt dd 0, 0
    flatdesc db 0xff, 0xff, 0, 0, 0, 10010010b, 11001111b, 0
    codedesc db 0xff, 0xff, 0, 0, 0, 10011010b, 11001111b, 0
    gdt_end:

kernelbin db "KERNEL.BIN", 0x3B, "1"
kernelbin_len equ ($ - kernelbin)