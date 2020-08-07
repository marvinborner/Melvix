; Melvin's awesome ext2 bootloader
; I'm not really good in assembly, there are MANY ways to improve this!
; MIT License, Copyright (c) 2020 Marvin Borner

; Definitions

%define LOCATION 0x7c00 ; Bootloader location

; General configurations
; TODO: Find out why 2560x1600 doesn't work
%define VIDEO_WIDTH 1920
%define VIDEO_HEIGHT 1200
%define VIDEO_BPP 4

; Interrupts
%define VIDEO_INT 0x10 ; Video BIOS Interrupt
%define DISK_INT 0x13 ; Disk BIOS Interrupt
%define MISC_INT 0x15 ; Miscellaneous services BIOS Interrupt

; Characters
%define NEWLINE 0x0A ; Newline character (\n)
%define RETURN 0x0D ; Return character (\r)
%define NULL 0x00 ; NULL character (\0)

; Video commands (VGA)
%define VIDEO_CLEAR 0x03 ; Clear screen command
%define VIDEO_OUT 0x0e ; Teletype output command

; Disk commands
%define DISK_EXT_CHECK 0x41 ; Disk extension check command
%define DISK_EXT_CHECK_SIG1 0x55aa ; First extension check signature
%define DISK_EXT_CHECK_SIG2 0xaa55 ; Second extension check signature
%define DISK_ZERO 0x80 ; First disk - TODO: Disk detection
%define DISK_READ 0x42 ; Disk extended read command

; EXT2 constants
%define EXT2_SB_SIZE 0x400 ; Superblock size
%define EXT2_SIG_OFFSET 0x38 ; Signature offset in superblock
%define EXT2_TABLE_OFFSET 0x08 ; Inode table offset after superblock
%define EXT2_INODE_TABLE_LOC 0x1000 ; New inode table location in memory
%define EXT2_KERNEL_INODE 0x05 ; Kernel inode
%define EXT2_INODE_SIZE 0x80 ; Single inode size
%define EXT2_GET_ADDRESS(inode) (EXT2_INODE_TABLE_LOC + (inode - 1) * EXT2_INODE_SIZE)
%define EXT2_COUNT_OFFSET 0x1c ; Inode offset of number of data blocks
%define EXT2_POINTER_OFFSET 0x28 ; Inode offset of first data pointer
%define EXT2_IND_POINTER_OFFSET 0x2c ; Inode offset of singly indirect data pointer
%define EXT2_DIRECT_POINTER_COUNT 0x0c ; Direct pointer count
%define EXT2_SIG 0xef53 ; Signature

; Video constants (VESA)
%define VESA_START 0x2000 ; Struct starts at 0x2000
%define VESA_END 0x3000 ; Struct ends at 0x3000
%define VESA_GET_MODES 0x4f00 ; Get video modes (via 10h)
%define VESA_GET_INFO 0x4f01 ; Get video mode info (via 10h)
%define VESA_SET_MODE 0x4f02 ; Set video mode (via 10h)
%define VESA_SUCCESS_SIG 0x004f ; Returns if VBE call succeeded
%define VESA_MODE_OFFSET 0xe ; Offset to mode pointer
%define VESA_MODE_SEGMENT 0x10 ; Mode pointer segment
%define VESA_LIST_END 0xffff ; End of mode list
%define VESA_PITCH_OFFSET 0x10 ; Pitch offset in mode info
%define VESA_WIDTH_OFFSET 0x12 ; Width offset in mode info
%define VESA_HEIGHT_OFFSET 0x14 ; Height offset in mode info
%define VESA_BPP_OFFSET 0x19 ; Bytes Per Pixel (BPP) offset in mode info
%define VESA_FRAMEBUFFER_OFFSET 0x2a ; Framebuffer offset in mode info
%define VESA_LFB_FLAG 0x4000 ; Enable LFB flag

; A20 constants
%define A20_GATE 0x92 ; Fast A20 gate
%define A20_ENABLED 0b10 ; Bit 1 defines whether A20 is enabled
%define A20_EXCLUDE_BIT 0xfe ; Bit 0 may be write-only, causing a crash

; GDT constants (bitmap)
%define GDT_MAX_LIMIT 0xffff ; I just use the max limit lel
%define GDT_PRESENT 0b10000000 ; Is present
%define GDT_RING3 0b01100000 ; Privilege level 3
%define GDT_DESCRIPTOR 0b00010000 ; Descriptor type, set for code/data
%define GDT_EXECUTABLE 0b00001000 ; Can be executed
%define GDT_READWRITE 0b00000010 ; Read/write access for code/data
%define GDT_ACCESSED 0b00000001 ; Whether segment is accessed
%define GDT_GRANULARITY (0x80 | 0x0f) ; Page granularity (4KiB)
%define GDT_SIZE (0x40 | 0x0f) ; Use 32 bit selectors
%define GDT_DATA_OFFSET 0x10 ; Offset to GDT data segment

; Kernel constants
%define STACK_POINTER 0x00900000 ; The initial stack pointer in kernel mode
%define KERNEL_POSITION 0x00050000 ; Loaded kernel position in protected mode (* 0x10)

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

; Now put some data and routines just before the end of the boot sector because
; we've still got some space left :)

; Video map routine
video_map:
	xor eax, eax
	mov es, ax
	mov bx, VESA_START ; Set load address
	mov di, bx
	mov ax, VESA_GET_MODES ; Get video modes
	int VIDEO_INT ; Ask BIOS for data!

	cmp ax, VESA_SUCCESS_SIG ; Check VBE support in response
	jne .error ; Not supported :(

	mov si, [bx + VESA_MODE_OFFSET] ; Mode pointer offset
	mov ax, [bx + VESA_MODE_SEGMENT] ; Mode pointer segment
	mov es, ax

	mov di, VESA_END ; End of VBE struct
.loop:
	mov bx, [es:si] ; Load bx with video mode
	cmp bx, VESA_LIST_END ; Is this the end?
	jae .done ; Yes, there aren't any modes left

	add si, 2
	mov [.mode], bx

	mov ax, VESA_GET_INFO ; Get mode information
	mov cx, [.mode] ; Save in here
	int VIDEO_INT ; BIOS interrupt!
	cmp ax, VESA_SUCCESS_SIG ; Check if call succeeded
	jne .error ; Nope, jump to error!

	xor ax, ax
	mov ax, [es:di + VESA_FRAMEBUFFER_OFFSET] ; Save framebuffer
	mov [.framebuffer], ax ; Move fb address to struct

	mov ax, [es:di + VESA_PITCH_OFFSET] ; Save pitch
	mov bx, [es:di + VESA_WIDTH_OFFSET] ; Save width
	mov cx, [es:di + VESA_HEIGHT_OFFSET] ; Save height
	mov dx, [es:di + VESA_BPP_OFFSET] ; Save BPP

	mov [.bpp], dx ; Move bpp to struct (bigger bpp is always desired)
	add di, 0x100

	cmp ax, [.pitch] ; Compare with desired pitch
	jne .loop ; Not equal, continue search!
	cmp bx, [.width] ; Compare with desired height
	jne .loop ; Not equal, continue search!
	cmp cx, [.height] ; Compare with desired height
	jne .loop ; Not equal, continue search!

	lea ax, [es:di - 0x100]
	mov [vid_info.array], ax
.set_mode:
	xor ax, ax
	mov ax, VESA_SET_MODE ; Set VBE mode
	mov bx, [.mode] ; Set mode address
	mov [vid_info], bx ; Move mode information to array
	or bx, VESA_LFB_FLAG ; Enable LFB
	int VIDEO_INT ; SET!
	cmp ax, VESA_SUCCESS_SIG ; Check if set succeeded
	jne .error ; Nope, jump to error!
.done:
	ret ; Finished loop and set!
.error: ; Something failed - print message and loop!
	mov si, video_error_msg
	call print
	jmp $

; Video default data
.mode dw 0
.width dw VIDEO_WIDTH
.height dw VIDEO_HEIGHT
.pitch dw (VIDEO_WIDTH * VIDEO_BPP)
.bpp dw VIDEO_BPP
.framebuffer dd 0

; Variables
disk_error_msg db "Disk error!", NEWLINE, RETURN, NULL
lba_error_msg db "LBA error!", NEWLINE, RETURN, NULL
video_error_msg db "Video error!", NEWLINE, RETURN, NULL
drive db 0

; Video info struct
vid_info:
.mode dd 0 ; Mode info pointer
.array dd 0 ; Mode array pointer

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
	; Verify signature
	mov ax, [superblock + EXT2_SIG_OFFSET]
	cmp ax, EXT2_SIG
	jne disk_error

	; Load inode table
	mov ax, [superblock + EXT2_SB_SIZE + EXT2_TABLE_OFFSET] ; Inode table
	shl ax, 1 ; Multiply ax by 2
	mov [lba], ax ; Sector
	mov ax, 2
	mov [count], ax ; Read 1024 bytes
	mov bx, EXT2_INODE_TABLE_LOC ; Copy data to 0x1000
	mov [dest], bx
	call disk_read

	; Load kernel
	mov bx, EXT2_GET_ADDRESS(EXT2_KERNEL_INODE) ; First block
	mov cx, [bx + EXT2_COUNT_OFFSET] ; Number of blocks for inode
	lea di, [bx + EXT2_POINTER_OFFSET] ; Address of first block pointer
	mov bx, 0x5000 ; Load to this address
	mov [dest + 2], bx
	mov bx, 0 ; Inode location = 0xF0000
	mov [dest], bx
	call kernel_load

	; Set video mode
	call video_map

	jmp protected_mode_enter

kernel_load:
	xor ax, ax ; Clear ax
	mov dx, ax ; Clear dx

	; TODO: Add singly pointer support (until ~12KiB)
	;cmp cx, EXT2_DIRECT_POINTER_COUNT ; Indirect pointer needed?
	;jge .indirect ; Singly indirect pointer

	mov ax, [di] ; Set ax = block pointer
	shl ax, 1 ; Multiply ax by 2
	mov [lba], ax
	mov [dest], bx
	call disk_read
	jmp .end

;.indirect:
;	push di
;	push bx
;	push cx
;
;	xor ebx, ebx
;
;	; Read singly indirect pointer
;	mov bx, EXT2_GET_ADDRESS(EXT2_KERNEL_INODE) ; First block
;	lea di, [bx + EXT2_IND_POINTER_OFFSET] ; Address of singly indirect pointer
;	mov bx, 0x3000 ; Arbitrary address
;	mov ax, [di] ; Set ax = block pointer
;	shl ax, 1 ; Multiply ax by 2
;	mov [lba], ax
;	mov [dest], bx
;	call disk_read
;
;	; Read data
;	sub cx, EXT2_DIRECT_POINTER_COUNT
;	lea di, [ebx + 4 * ecx]
;	mov bx, 0x4000 ; Arbitrary address
;	mov ax, [di]
;	shl ax, 1
;	;sub bx, 0x400
;	mov [lba], ax
;	mov [dest], bx
;	call disk_read
;
;	pop cx
;	pop bx
;	pop di

.end:
	add bx, 0x400 ; 1kb increase
	add di, 0x4 ; Move to next block pointer
	sub cx, 0x2 ; Read 2 blocks
	jnz kernel_load
	ret

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
	mov ax, 0x28
	ltr ax

	; Set protected mode via cr0
	mov eax, cr0
	or eax, 1 ; Set bit 0
	mov cr0, eax

	jmp 08h:protected_mode ; JUMP!

bits 32 ; Woah, so big!
protected_mode:
	mov ax, GDT_DATA_OFFSET ; Data segment offset of GDT
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax ; Stack segment

	mov esp, STACK_POINTER ; Move stack pointer

	mov eax, vid_info ; Pass VBE struct to kernel
	push eax ; Push as second kernel parameter

	mov edx, KERNEL_POSITION
	lea eax, [edx]
	call eax

; TSS
tss_entry:
	dd 0 ; Previous TSS
	dd STACK_POINTER ; esp0
	dd GDT_DATA_OFFSET ; ss0
	dd 0 ; esp1
	dd 0 ; ss1
	dd 0 ; esp2
	dd 0 ; ss2
	dd 0 ; cr3
	dd 0 ; eip
	dd 0 ; eflags
	dd 0 ; eax
	dd 0 ; ecx
	dd 0 ; edx
	dd 0 ; ebx
	dd 0 ; esp
	dd 0 ; ebp
	dd 0 ; esi
	dd 0 ; edi
	dd 0x13 ; es
	dd 0x0b ; cs
	dd 0x13 ; ss
	dd 0x13 ; ds
	dd 0x13 ; fs
	dd 0x13 ; gs
	dd 0 ; ldt
	dw 0 ; trap
	dw 0 ; iomap base
tss_entry_end:

; GDT
align 32
gdt: ; GDTs start
gdt_null: ; Must be null
	dd 0
	dd 0
gdt_code: ; Code segment
	dw GDT_MAX_LIMIT ; Limit
	dw 0 ; First base
	db 0 ; Second base
	db (GDT_PRESENT | GDT_DESCRIPTOR | GDT_EXECUTABLE | GDT_READWRITE) ; Configuration
	db (GDT_GRANULARITY | GDT_SIZE) ; Flags
	db 0 ; Third base
gdt_data: ; Data segment
	dw GDT_MAX_LIMIT ; Limit
	dw 0 ; First base
	db 0 ; Second base
	db (GDT_PRESENT | GDT_DESCRIPTOR | GDT_READWRITE) ; Configuration
	db (GDT_GRANULARITY | GDT_SIZE) ; Flags
	db 0 ; Third base
gdt_user_code: ; User code segment
	dw GDT_MAX_LIMIT ; Limit
	dw 0 ; First base
	db 0 ; Second base
	db (GDT_PRESENT | GDT_RING3 | GDT_DESCRIPTOR | GDT_EXECUTABLE | GDT_READWRITE) ; Configuration
	db (GDT_GRANULARITY | GDT_SIZE) ; Flags
	db 0 ; Third base
gdt_user_data: ; Data segment
	dw GDT_MAX_LIMIT ; Limit
	dw 0 ; First base
	db 0 ; Second base
	db (GDT_PRESENT | GDT_RING3 | GDT_DESCRIPTOR | GDT_READWRITE) ; Configuration
	db (GDT_GRANULARITY | GDT_SIZE) ; Flags
	db 0 ; Third base
gdt_tss: ; TSS segment
	dw tss_entry + (tss_entry_end - tss_entry) ; Limit
	dw tss_entry ; First base
	db 0 ; Second base
	db (GDT_PRESENT | GDT_RING3 | GDT_EXECUTABLE | GDT_ACCESSED) ; Configuration
	db GDT_SIZE ; Flags
	db 0 ; Third base
gdt_end:
gdt_desc:
	dw gdt_end - gdt - 1
	dd gdt

times 1024 - ($ - $$) db 0

; Start at LBA 2
superblock:
