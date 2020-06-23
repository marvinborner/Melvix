bits 16
org 0x7c00

global _start
_start:
	; Clear screen
	mov ax, 0x003
	int 0x10

	; Welcome user!
	mov si, hello_msg
	call print

	; Check LBA support
	mov ah, 0x41
	mov bx, 0x55AA
	int 0x13
	jc lba_error
	cmp bx, 0xAA55
	jnz lba_error

	; Check disk and move dl
	and dl, 0x80 ; Use disk 0
	jz disk_error
	mov [drive], dl

	; Load stage two
	mov bx, stage_two
	mov [dest], bx
	call disk_read

	; JUMP
	jmp stage_two

print:
	mov ah, 0x0E
	xor bh, bh
	print_ch:
		lodsb
		test al, al
		jz print_end
		int 0x10
		jmp print_ch
	print_end:
	ret

disk_read:
	mov si, packet ; Address of dap
	mov ah, 0x42 ; Extended read
	mov dl, [drive] ; Drive number
	int 0x13
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
hello_msg db "Welcome! Loading Melvix...", 0x0A, 0x0D, 0x00
disk_error_msg db "Disk error!", 0x0a, 0x0d, 0x00
lba_error_msg db "LBA error!", 0x0a, 0x0d, 0x00
stage_two_msg db "Stage2 loaded", 0x0a, 0x0d, 0x00
disk_success_msg db "Disk is valid", 0x0a, 0x0d, 0x00
inode_table_msg db "Found inode table", 0x0a, 0x0d, 0x00
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
	mov ax, [superblock +56]
	cmp ax, 0xEF53
	jne disk_error
	mov si, disk_success_msg
	call print

	; load inode table
	mov ax, [superblock + 1024 + 8] ; Inode table
	mov cx, 2
	mul cx ; ax = cx * ax
	mov [lba], ax ; Sector
	mov ax, 2
	mov [count], ax ; Read 1024 bytes
	mov bx, 0x1000 ; Copy data to 0x1000
	mov [dest], bx
	call disk_read
	mov si, inode_table_msg
	call print

	; Load root dir
	xor bx, bx
	mov ax, bx
	mov bx, 0x1080 ; First block ((2 - 1) * 128 + dest)
	mov ax, word [bx + 0] ; Filetype
	and ax, 0x4000 ; Check if dir
	cmp ax, 0x4000
	jne disk_error
	mov cx, [bx + 28] ; Number of sectors for inode
	lea di, [bx + 40] ; Address of first block pointer

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

	add bx, 0x400 ; 1kb increase
	add di, 0x4 ; Move to next block pointer
	sub cx, 0x2 ; Read 2 blocks
	jnz stage_three_load
	ret

	nop
	hlt

protected_mode_enter:
	cli ; Turn off interrupts

	; TODO: Check A20 support?
	in al, 0x92 ; Enable A20
	or al, 2
	out 0x92, al

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
