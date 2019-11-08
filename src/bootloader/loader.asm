# Stolen from https://github.com/jlxip/jotadOS/tree/master/src/JBoot
# License: https://github.com/jlxip/jotadOS/blob/master/LICENSE

BITS 16
ORG 0x7C00

; First, save the boot drive ID, which is in DL.
; This way, we will know where we're booting from.
; We will only use that drive.
mov [bootDriveID], dl

jmp start

; Some neat functions and constants.
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
    dapack_size:            db  0x10
    dapack_null:            db  0x00
    dapack_blkcount:        dw  0x0001	; 1 block = 1 sector (2K in ATAPI)
    dapack_boffset:         dw  0x9000
    dapack_bsegment:        dw  0x0000
    dapack_start:           dd  0x00000000
    dapack_upper_lba_bits:  dd  0x00000000

readsector:
	; Input:  eax = LBA
	; Output: blocks starting at dapack_boffset
	mov dword [dapack_start], eax

	; Invoke the interrupt
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
	; Input:  [esp+2] = directory record address
	;         [esp+4] = length of filename (only low byte used)
	;         [esp+6] = filename*
	; Output: In case of success: ax = 0, bx = (start of directory record)
	;         In case of failure: ax = 1

	; First, save the data in the stack
	mov bx, [esp+2]
	mov ax, [esp+4]
	mov [filenameLength], ax
	mov ax, [esp+6]
	mov [filename], ax

	.findfile_L:
		; Get the size (al)
		mov al, [bx]
		; If it's zero, the kernel is not there.
		test al, al
		jz .findfile_notfound

		; Check if filename length matches.
		mov ah, [bx+32]
		cmp ah, byte [filenameLength]
		; If they don't match, keep seeking.
		jnz .findfile_keep

		; At this point, they do match. Now compare the strings.
		call findfile_check
		test ax, ax
		jz findfile_found

		; The filenames don't match.
		.findfile_keep:
		mov al, [bx]
		xor ah, ah
		add bx, ax
		jmp .findfile_L

	.findfile_notfound:
		; Not found. ax = 1
		xor ax, ax
		inc ax
		ret

	findfile_check:
		; Compares the filename.
		; Returns 0 in case of success, 1 otherwise.
		pusha
		add bx, 33
		mov ax, bx
		; ax is the base of the string in the CD.

		xor cx, cx
		.findfile_check_L:
			; First character.
			mov bx, [filename]
			add bx, cx
			mov dh, [bx]
			; Second character.
			mov bx, ax
			add bx, cx
			mov dl, [bx]
			; Compare.
			cmp dh, dl
			jnz .findfile_check_fail

			; At this point, the characters match.
			; Are we done?
			inc cx
			cmp cx, word [filenameLength]
			jz .findfile_check_success	; Yes
			jmp .findfile_check_L	; Nope

			.findfile_check_fail:
			popa
			xor ax, ax
			inc ax
			ret
			.findfile_check_success:
			; We're done.
			popa
			xor ax, ax
			ret

	findfile_found:
		; Found. ax = 0
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
	; Source: https://wiki.osdev.org/A20_Line
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
nolba db "BIOS lacks support for LBA addressing.", 0x00
noboot db "Boot directory could not be found.", 0x00
noa20 db "A20 could not be enabled.", 0
loading db "Loading kernel...", 0x0A, 0x0D, 0x00
nokernel db "kernel.bin could not be found!", 0
booting db "Booting...", 0x0A, 0x0D, 0x00
nomem db "BIOS does not support memory detection!", 0
memno20 db "BIOS returns memory detection with 24 bytes. This has never been seen!", 0

start:
; Clear screen.
mov ax, 0x0003
int 0x10

; Print welcome.
mov si, welcome
call print

; Check if LBA is supported by the BIOS.
mov ah, 0x41
mov bx, 0x55AA
int 0x13
jc lba_not_supported
cmp bx, 0xAA55
jnz lba_not_supported

; LBA is supported at this point.

; Now, check whether A20 is enabled.
call checkA20
test ax, ax
jnz A20_ENABLED

; It's not enabled. Enable it through "Fast A20 Gate".
in al, 0x92
or al, 2
out 0x92, al

; Check if it's enabled now.
call checkA20
test ax, ax
jnz A20_ENABLED

; It didn't work. Too bad for the user.
mov si, noa20
call print
jmp $

A20_ENABLED:
; A20 is enabled at this point.

; Enter Big Unreal Mode (to write past the 1M barrier).
; Thanks to: https://wiki.osdev.org/Unreal_Mode
cli	; Disable interrupts
push ds	; Save real mode
lgdt [gdtinfo]	; Load the temporal GDT

mov eax, cr0	; Switch to protected mode
or al, 1
mov cr0, eax

jmp $+2	; Tell 386/486 to not crash (y tho?)

mov bx, 0x08	; Select descriptor 1
mov ds, bx

and al, 0xFE	; Back to real mode
mov cr0, eax
pop ds
; We are now in Big Unreal Mode.

; About to begin the real shit.
mov si, loading
call print



; Load Primary Volume Descriptor onto memory.
call LOAD_PVD
; PVD is now @ 0x9000

; Load the root directory.
mov bx, 0x9000	; Base
add bx, 156	; Directory Record of root
add bx, 2	; LBA
mov eax, dword [bx]
call readsector

; Find boot directory.
push boot
push boot_len
push 0x9000
call findfile
add esp, 6
test ax, ax
jz continue_BOOT

; No boot directory.
mov si, noboot
call print
jmp $

continue_BOOT:
; Directory record address @ bx.
; Load boot directory record.
mov word [dapack_blkcount], 0x0001	; The boot directory is not so big.
add bx, 2	; Extent
mov eax, [bx]
call readsector

; Find "kernel.bin".
push kernelbin
push kernelbin_len
push 0x9000
call findfile
add esp, 6
test ax, ax
jz continue_KERNEL

; No kernel?
mov si, nokernel
call print
jmp $

continue_KERNEL:
; The kernel directory record is now @ bx.

; Bear in mind that we can't read the ELF directly, as BIOS interrupts run on real mode.
; Instead, we'll be loading one block at a time (2K) to 0x9000.
mov word [dapack_blkcount], 1
mov word [dapack_boffset], 0x9000

; We'll load the ELF at 2M, and then parse it and load the kernel at 1M.
; This will work as long as the kernel is below 1M of size. If that point ever
; comes, just change 0x200000 to 0x300000 or something.

; Get the size of the ELF in blocks.
push bx
add bx, 10		; Offset for size.
mov eax, [bx]	; Size in bytes
xor edx, edx	; Convert to blocks
mov ebx, 2048
div ebx
inc eax	; Round up
pop bx
push eax	; Save the number of blocks to read.

add bx, 2
mov eax, [bx]	; Starting LBA of the ELF.
push eax	; Save it too.

; Offsetless count.
xor ecx, ecx
LOAD_KERNEL:
	; Compute starting LBA for current block and read it.
	mov eax, [esp]	; Offset
	add eax, ecx	; + current offsetless LBA
	call readsector

	; Calculate the current block's starting position.
	mov eax, ecx	; Current offsetless LBA
	shl eax, 11	; * 2048
	add eax, 0x200000	; + 2M

	; Move the block. I couldn't get "movs" working.
	push ecx
	xor ecx, ecx
	.LOAD_KERNEL_L:
		; Get current doubleword.
		mov ebx, ecx
		add ebx, 0x9000
		mov ebx, dword [ebx]

		; Move it.
		mov edx, ebx
		mov ebx, eax
		add ebx, ecx
		mov dword [ebx], edx

		; Go for the next one.
		add ecx, 4
		cmp ecx, 2048
		jl .LOAD_KERNEL_L
	pop ecx

	inc ecx
	cmp ecx, dword [esp+4]
	jl LOAD_KERNEL

; The whole ELF is now in memory!
mov si, booting
call print

; Get the kernel what it needs. We'll put all of this at 0x9000.
; 0x9000: boot drive ID (byte)
mov dl, [bootDriveID]	; Reference to stage 1
mov byte [0x9000], dl

; 0xA000: available RAM (dword)
; We'll do it by BIOS function 0x15, eax=0xE820
mov di, 0xA000
mov eax, 0xE820
xor ebx, ebx
mov ecx, 24
mov edx, 0x534D4150
int 0x15

; Check if everything went fine (BIOS supports it).
jc BIOS_NO_MEM
cmp eax, 0x534D4150
jnz BIOS_NO_MEM
cmp cl, 20
jnz BIOS_MEM_NO20

; Go for the next entries.
MEM_L:
	; Are we done?
	test ebx, ebx
	jz MEM_FINISHED

	; Nope. Go for the next one.
	mov ax, di
	xor ch, ch
	add ax, cx
	mov di, ax

	mov eax, 0xE820
	mov ecx, 24
	int 0x15
	jmp MEM_L
MEM_FINISHED:
; The list is now at 0xA000.

; Enter protected mode
mov eax, cr0
or al, 1
mov cr0, eax

mov ax, 0x08	; Select descriptor 1
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax

; Everything set. Now it's time to parse the ELF.
; jotadOS is x86, so I'll follow those specs.
; Guidance: https://wiki.osdev.org/ELF

; First, get the number of entries in the program header table (offset +44, byte)
mov ebx, 0x20002C
mov dl, byte [ebx]
push dx	; Save it in the stack
; Now, the size of each one is 32 bits. Because the ELF contains 32 bit instructions.

; Get the start of the program header.
mov ebx, 0x20001C
mov ebx, dword [ebx]
push ebx

; Iterate thru each one
xor dh, dh
PHT:
	xor eax, eax
	mov al, dh	; Current entry
	shl eax, 5	; * 32 (size)
	add eax, dword [esp]	; + Start
	add eax, 0x200000	; + Memory offset
	mov ebx, eax
	; It's now at ebx.

	; Check that the type of segment is 1 (load).
	mov eax, dword [ebx]
	cmp eax, 1
	jnz .PHT_ignore

	; At this point, type type is 1.
	; We have to "copy p_filesz bytes from p_offset to p_vaddr".

	; Get "p_offset" (offset +4, dword) into the stack.
	add ebx, 4
	mov eax, dword [ebx]	; p_offset
	add eax, 0x200000	; + Memory offset
	push eax

	; Get "p_vaddr" (offset +8, dword) as well.
	add ebx, 4
	mov eax, dword [ebx]	; p_vaddr
	push eax

	; Finally, "p_filesz" (offset +16, dword).
	add ebx, 8
	mov eax, dword [ebx]
	push eax

	; Move the data!
	push dx
	xor ecx, ecx
	.MOVE_KERNEL_L:
		; Get current doubleword.
		mov ebx, [esp+10]
		add ebx, ecx
		mov ebx, dword [ebx]

		; Move it.
		mov eax, ebx
		mov ebx, dword [esp+6]
		add ebx, ecx
		mov dword [ebx], eax

		; Go for the next one.
		add ecx, 4
		cmp ecx, dword [esp+2]
		jl .MOVE_KERNEL_L
	pop dx
	add esp, 12

	.PHT_ignore:
	; Next one!
	inc dh
	cmp dh, dl
	jl PHT

; Everything set. Use the section at the beginning to locate
; the entry point ('_start').

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
	dw gdt_end - gdt - 1	; Size of the table
	dd gdt	; Its start
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