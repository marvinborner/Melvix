org 0x800
bits 16

KERNEL equ 0x1000
KERNEL_SECTORS equ 24

call load_kernel

int 0x12
mov [0x600], ax

call switch_to_pm

gtd_start:
gdt_null:
    dd 0x0
    dd 0x0

gdt_code:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0

gdt_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

switch_to_pm:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:init_pm

bits 32
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, 0x90000
    call begin_pm

load_kernel:
    mov ax, 3
    mov cl, 4
    mov ch, 0
    mov bx, KERNEL
    mov dl, [BOOT_DRIVE]
    mov dh, 0
    mov ch, 0
load_sector:
    mov ah, 0x02
    mov al, 1
    int 0x13
    push bx
    mov bl, [Sector]
    cmp bl, KERNEL_SECTORS
    pop bx
    je loaded
    push bx
    mov bl, [Sector]
    inc bl
    mov [Sector], bl
    pop bx
    inc cl
    cmp cl, 18
    jne continue
    add ch, 1
    add ch, 1
    mov cl, 1
continue:
    add bx, BytesPerSector
    jmp load_sector
loaded:
    ret

begin_pm:
    call KERNEL
    jmp $

BytesPerSector equ 512
NumHeads equ 2
SectorsPerTrack equ 18
Sector db 0

BOOT_DRIVE db 0

times 1024-($-$$) db 0
