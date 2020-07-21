; Melvin's awesome ext2 bootloader
; I'm not really good in assembly, there are MANY ways to improve this!
; MIT License, Copyright (c) 2020 Marvin Borner

; Definitions
%define LOCATION 0x7c00 ; Bootloader location

%define NEWLINE 0x0A ; Newline character (\n)
%define RETURN 0x0D ; Return character (\r)
%define NULL 0x00 ; NULL character (\0)

%define VIDEO_INT 0x10 ; Video BIOS Interrupt
%define VIDEO_CLEAR 0x03 ; Clear screen command
%define VIDEO_OUT 0x0e ; Teletype output command

%define DISK_INT 0x13 ; Disk BIOS Interrupt
%define DISK_EXT_CHECK 0x41 ; Disk extension check command
%define DISK_EXT_CHECK_SIG1 0x55aa ; First extension check signature
%define DISK_EXT_CHECK_SIG2 0xaa55 ; Second extension check signature
%define DISK_ZERO 0x80 ; First disk - TODO: Disk detection
%define DISK_READ 0x42 ; Disk extended read command

%define EXT2_SB_SIZE 0x400 ; Superblock size
%define EXT2_SIG_OFFSET 0x38 ; Signature offset in superblock
%define EXT2_TABLE_OFFSET 0x08 ; Inode table offset after superblock
%define EXT2_INODE_TABLE_LOC 0x1000 ; New inode table location in memory
%define EXT2_KERNEL_INODE 0x05 ; Kernel inode
%define EXT2_INODE_SIZE 0x80 ; Single inode size
%define EXT2_GET_ADDRESS(inode) (EXT2_INODE_TABLE_LOC + (inode - 1) * EXT2_INODE_SIZE)
%define EXT2_COUNT_OFFSET 0x1c ; Inode offset of number of data blocks
%define EXT2_POINTER_OFFSET 0x28 ; Inode offset of first data pointer
%define EXT2_SIG 0xef53 ; Signature

%define STACK_POINTER 0x00900000 ; The initial stack pointer in kernel mode
%define KERNEL_POSITION 0x00050000 ; Loaded kernel position in protected mode (* 0x10)

%define A20_GATE 0x92 ; Fast A20 gate
%define A20_ENABLED 0b10 ; Bit 1 defines whether A20 is enabled
%define A20_EXCLUDE_BIT 0xfe ; Bit 0 may be write-only, causing a crash
; ENOUGH, let's go!

bits 16
org LOCATION

; This is the first stage. It prints some things, checks some things
; and jumps to the second stage. Nothing special.
global _start
_start:
	; Clear screen
	mov ax, VIDEO_CLEAR
	int VIDEO_INT

	; Welcome user!
	mov si, hello_msg
	call print

	; Check LBA support
	mov ah, DISK_EXT_CHECK
	mov bx, DISK_EXT_CHECK_SIG1
	int DISK_INT
	jc lba_error
	cmp bx, DISK_EXT_CHECK_SIG2
	jnz lba_error

	; Check disk and move dl
	and dl, DISK_ZERO ; Use disk 0
	jz disk_error
	mov [drive], dl

	; Load stage two
	mov bx, stage_two
	mov [dest], bx
	call disk_read

	; JUMP
	jmp stage_two

print:
	push bx
	push ax
	mov ah, VIDEO_OUT
	xor bh, bh
	print_ch:
		lodsb
		test al, al
		jz print_end
		int VIDEO_INT
		jmp print_ch
	print_end:
	pop ax
	pop bx
	ret

disk_read:
	mov si, packet ; Address of dap
	mov ah, DISK_READ ; Extended read
	mov dl, [drive] ; Drive number
	int DISK_INT
	jc disk_error
	ret

; Errors
disk_error:
	mov si, disk_error_msg
	call print
	jmp $
lba_error:
	mov si, lba_error_msg
	call print
	jmp $

; Variables
hello_msg db "Welcome! Loading Melvix...", NEWLINE, RETURN, NULL
disk_error_msg db "Disk error!", NEWLINE, RETURN, NULL
lba_error_msg db "LBA error!", NEWLINE, RETURN, NULL
stage_two_msg db "Stage2 loaded", NEWLINE, RETURN, NULL
disk_success_msg db "Disk is valid", NEWLINE, RETURN, NULL
inode_table_msg db "Found inode table", NEWLINE, RETURN, NULL
protected_msg db "Jumping to protected mode", NEWLINE, RETURN, NULL
drive db 0

; Data
packet:
	db 0x10 ; Packet size
	db 0 ; Always 0
count:
	dw 4 ; Number of sectors to transfer
dest:
	dw 0 ; Destination offset
	dw 0 ; Destination segment
lba:
	dd 1 ; LBA number
	dd 0 ; More storage bytes

; End of boot sector
times 510 - ($ - $$) db 0
dw 0xAA55

; This is the second stage. It tries to load the kernel (inode 5) into memory.
; To do this, it first checks the integrity of the ext2 fs. Then it has to find
; the address of the fifth inode and load its contents into memory.
; After this is finished, the stage can jump into the protected mode, enable the
; A20 line and finally jump to the kernel! ez
stage_two:
	mov si, stage_two_msg
	call print ; yay!

	; Verify signature
	mov ax, [superblock + EXT2_SIG_OFFSET]
	cmp ax, EXT2_SIG
	jne disk_error
	mov si, disk_success_msg
	call print

	; Load inode table
	mov ax, [superblock + EXT2_SB_SIZE + EXT2_TABLE_OFFSET] ; Inode table
	shl ax, 1 ; Multiply ax by 2
	mov [lba], ax ; Sector
	mov ax, 2
	mov [count], ax ; Read 1024 bytes
	mov bx, EXT2_INODE_TABLE_LOC ; Copy data to 0x1000
	mov [dest], bx
	call disk_read
	mov si, inode_table_msg
	call print

	; Load kernel
	mov bx, EXT2_GET_ADDRESS(EXT2_KERNEL_INODE) ; First block
	mov cx, [bx + EXT2_COUNT_OFFSET] ; Number of sectors for inode
	lea di, [bx + EXT2_POINTER_OFFSET] ; Address of first block pointer
	mov bx, 0x5000 ; Load to this address
	mov [dest + 2], bx
	mov bx, 0 ; Inode location = 0xF0000
	mov [dest], bx
	call kernel_load

	jmp protected_mode_enter

kernel_load:
	xor ax, ax ; Clear ax
	mov dx, ax ; Clear dx
	mov ax, [di] ; Set ax = block pointer
	shl ax, 1 ; Multiply ax by 2

	mov [lba], ax
	mov [dest], bx

	call disk_read

	add bx, 0x400 ; 1kb increase
	add di, 0x4 ; Move to next block pointer
	sub cx, 0x2 ; Read 2 blocks
	jnz kernel_load
	ret

protected_mode_enter:
	cli ; Turn off interrupts
	mov si, protected_msg
	call print

	; TODO: Check A20 support?
	; TODO: 0x92 method may not work on every device
	in al, A20_GATE
	test al, A20_ENABLED
	jnz .a20_enabled
	or al, A20_ENABLED
	and al, A20_EXCLUDE_BIT
	out A20_GATE, al
	.a20_enabled:

	; Clear registers
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	lgdt [gdt_desc] ; Load GDT

	; Set protected mode via cr0
	mov eax, cr0
	or eax, 1 ; Set bit 0
	mov cr0, eax

	jmp 08h:protected_mode ; JUMP!

bits 32 ; Woah, so big!
protected_mode:
	mov ax, 10h ; Set data segement indentifier
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax ; Stack segment

	mov esp, STACK_POINTER ; Move stack pointer

	mov edx, KERNEL_POSITION
	lea eax, [edx]
	call eax

; GDT
align 32
gdt:
gdt_null:
	dd 0
	dd 0
gdt_code:
	dw 0xFFFF
	dw 0
	db 0
	db 0x9A
	db 0xCF
	db 0
gdt_data:
	dw 0xFFFF
	dw 0
	db 0
	db 0x92
	db 0xCF
	db 0
gdt_end:
gdt_desc:
	dw gdt_end - gdt - 1
	dd gdt

times 1024 - ($ - $$) db 0

; Start at LBA 2
superblock:
