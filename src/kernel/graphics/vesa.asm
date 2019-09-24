global vbe_set_mode
global vbe_find_mode

vbe_set_mode:
	mov [width], ax
	mov [height], bx
	mov [bpp], cl

	sti

    ; Get VBE BIOS info
	push es
	mov ax, 0x4F00
	mov di, [vbe_info]
	int 0x10
	pop es

    ; Check if BIOS has VBE support
	cmp ax, 0x4F
	jne error

	mov ax, word[vbe_info.video_modes]
	mov [offset], ax
	mov ax, word[vbe_info.video_modes+2]
	mov [mode_segment], ax

	mov ax, [mode_segment]
	mov fs, ax
	mov si, [offset]

vbe_find_mode:
	mov dx, [fs:si]
	add si, 2
	mov [offset], si
	mov [mode], dx
	mov ax, 0
	mov fs, ax

	cmp [mode], word 0xFFFF
	je error

    ; Get VBE mode info
	push es
	mov ax, 0x4F01
	mov cx, [mode]
	mov di, [vbe_mode_info]
	int 0x10
	pop es

	cmp ax, 0x4F
	jne error

	mov ax, [width]
	cmp ax, [vbe_mode_info.width]
	jne next_mode

	mov ax, [height]
	cmp ax, [vbe_mode_info.height]
	jne next_mode

	mov al, [bpp]
	cmp al, [vbe_mode_info.bpp]
	jne next_mode

	; Found best mode!
	mov ax, [width]
	mov word[vbe_best.width], ax
	mov ax, [height]
	mov word[vbe_best.height], ax
	mov eax, [vbe_mode_info.framebuffer]
	mov dword[vbe_best.framebuffer], eax
	mov ax, [vbe_mode_info.pitch]
	mov word[vbe_best.bytes_per_line], ax
	mov eax, 0
	mov al, [bpp]
	mov byte[vbe_best.bpp], al
	shr eax, 3
	mov dword[vbe_best.bytes_per_pixel], eax

	mov ax, [width]
	shr ax, 3
	dec ax
	mov word[vbe_best.x_cur_max], ax

	mov ax, [height]
	shr ax, 4
	dec ax
	mov word[vbe_best.y_cur_max], ax

	; Set the mode
	push es
	mov ax, 0x4F02
	mov bx, [mode]
	or bx, 0x4000
	mov di, 0
	int 0x10
	pop es

	cmp ax, 0x4F
	jne error

	clc
	ret

next_mode:
	mov ax, [mode_segment]
	mov fs, ax
	mov si, [offset]
	jmp vbe_find_mode

error:
	stc
	ret

width				dw 0
height				dw 0
bpp				db 0
mode_segment			dw 0
offset				dw 0
mode				dw 0

vbe_info:
	.signature					db "VESA"
	.version					dw 0
	.oem						dd 0
	.capabilities 				dd 0
	.video_modes				dd 0
	.video_memory				dw 0
	.software_rev				dw 0
	.vendor						dd 0
	.product_name				dd 0
	.product_rev				dd 0
	.reserved					times 222 db 0
	.oem_data					times 256 db 0

vbe_mode_info:
	.attributes					dw 0
	.window_a					db 0
	.window_b					db 0
	.granularity				dw 0
	.window_size				dw 0
	.segment_a					dw 0
	.segment_b					dw 0
	.win_func_ptr			 	dd 0
	.pitch						dw 0
	.width						dw 0
	.height						dw 0
	.w_char						db 0
	.y_char						db 0
	.planes						db 0
	.bpp						db 0
	.banks						db 0
	.memory_model				db 0
	.bank_size					db 0
	.image_pages				db 0
	.reserved0					db 0

	.red_mask					db 0
	.red_position				db 0
	.green_mask					db 0
	.green_position				db 0
	.blue_mask					db 0
	.blue_position				db 0
	.reserved_mask				db 0
	.reserved_position			db 0
	.direct_color_attributes	db 0

	.framebuffer				dd 0
	.off_screen_mem_off			dd 0
	.off_screen_mem_size		dw 0
	.reserved1					times 206 db 0

vbe_best:
	.bpp db 0
	.height dw 0
	.width dw 0
	.mode dw 0
	.framebuffer dd 0
	.bytes_per_line dw 0
	.bytes_per_pixel dd 0
	.x_cur_max dw 0
	.y_cur_max dw 0