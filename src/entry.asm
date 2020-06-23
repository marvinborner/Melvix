; Melvin's awesome ext2 bootloader
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
%define EXT2_NEW_TABLE 0x1000 ; New inode table location in memory
%define EXT2_INODE_SIZE 0x80 ; Single inode size
%define EXT2_ROOT_INODE 0x02 ; Root directory inode
%define EXT2_ROOT_DIR EXT2_NEW_TABLE + (EXT2_ROOT_INODE - 1) * EXT2_INODE_SIZE
%define EXT2_TYPE_OFFSET 0x00 ; Offset of filetype and rights
%define EXT2_COUNT_OFFSET 0x1c ; Offset of number of data blocks
%define EXT2_POINTER_OFFSET 0x28 ; Offset of first data pointer
%define EXT2_SIG 0xef53 ; Signature
%define EXT2_DIR 0x4000 ; Directory indicator
%define EXT2_REG 0x8000 ; Regular file indicator

%define A20_GATE 0x92 ; Fast A20 gate
%define A20_ENABLED 0b10 ; Bit 1 defines whether A20 is enabled
%define A20_EXCLUDE_BIT 0xfe ; Bit 0 may be write-only, causing a crash

; ENOUGH, let's go!

bits 16
org LOCATION

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
	mov ah, VIDEO_OUT
	xor bh, bh
	print_ch:
		lodsb
		test al, al
		jz print_end
		int VIDEO_INT
		jmp print_ch
	print_end:
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
	mov cx, 2
	mul cx ; ax = cx * ax
	mov [lba], ax ; Sector
	mov ax, 2
	mov [count], ax ; Read 1024 bytes
	mov bx, EXT2_NEW_TABLE ; Copy data to 0x1000
	mov [dest], bx
	call disk_read
	mov si, inode_table_msg
	call print

	; Load root dir
	xor bx, bx
	mov ax, bx
	mov bx, EXT2_ROOT_DIR ; First block
	mov ax, word [bx + EXT2_TYPE_OFFSET] ; Filetype
	and ax, EXT2_DIR ; Check if directory
	cmp ax, EXT2_DIR
	jne disk_error
	mov cx, [bx + EXT2_COUNT_OFFSET] ; Number of sectors for inode
	lea di, [bx + EXT2_POINTER_OFFSET] ; Address of first block pointer
	jmp $

	; Find kernel
	xor ax, ax
	mov dx, ax
	mov ax, [di]
	mov dx, 2
	mul dx
	mov [lba], ax
	mov [dest], bx

	mov bx, 0x5000
	mov [dest + 2], bx
	mov bx, 0 ; Inode location = 0xF0000
	mov [dest], bx
	call stage_three_load

	jmp protected_mode_enter

stage_three_load:
	xor ax, ax ; Clear ax
	mov dx, ax ; Clear dx
	mov ax, [di] ; Set ax = block pointer
	mov dx, 2 ; Mul 2 for sectors
	mul dx ; ax = dx * ax

	mov [lba], ax
	mov [dest], bx

	call disk_read

	add bx, 1024 ; 1kb increase
	add di, 0x4 ; Move to next block pointer
	sub cx, 2 ; Read 2 blocks
	jnz stage_three_load
	ret

	nop
	hlt

protected_mode_enter:
	cli ; Turn off interrupts

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

	mov eax, cr0
	or eax, 1 ; Set bit 0
	mov cr0, eax

	jmp 08h:protected_mode ; JUMP!

bits 32 ; Woah!
protected_mode:
	xor eax, eax

	mov ax, 10h ; Set data segement indentifier
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax ; Stack segment
	mov esp, 0x00900000 ; Move stack pointer

	mov edx, 0x00050000
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
	db 0xcF
	db 0
gdt_end:
gdt_desc:
	dw gdt_end - gdt - 1
	dd gdt

times 1024 - ($ - $$) db 0

superblock:
