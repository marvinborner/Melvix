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
	and dl, 0x80
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
	call print

	mov ax, [superblock +56]
	cmp ax, 0xEF53
	jne disk_error

	mov si, disk_success_msg
	call print

	jmp $


times 1024 - ($ - $$) db 0
superblock:
