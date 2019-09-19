global find_mode
global best_video_mode

vbe_info_block:
	.signature					db "VBE2"	; indicate support for VBE 2.0+
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

mode_info_block:
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

best_video_mode:
	.bpp db 0
	.height dw 0
	.width dw 0
	.mode dw 0
	.framebuffer dd 0
	.bytes_per_line dw 0
	.bytes_per_pixel dd 0
	.x_cur_max dw 0
	.y_cur_max dw 0

segments dw 0
offset  dw 0
mode 	dw 0

width equ 1024
height equ 768
bpp equ 32

search_video_mode:
    [bits 16]

    push es
    mov ax, 0x4F00
    mov di, vbe_info_block
    int 0x10
    pop es

    mov ax, word [vbe_info_block.video_modes]
    mov [offset], ax
    mov ax, word [vbe_info_block.video_modes+2]
    mov [segments], ax

    mov ax, [segments]
    mov fs, ax
    mov si, [offset]

    mov cx, 1
    push cx

find_mode:
	mov dx, [fs:si]
	add si, 2
	mov [offset], si
	mov [mode], dx
	mov ax, 0
	mov fs, ax

	push es
	mov ax, 0x4F01
	mov cx, [mode]
	mov di, mode_info_block
	int 0x10
	pop es

	cmp ax, 0x004F

	pop cx
	cmp cx, 4
	je reset_counter
	inc cx
	push cx

	check_mode:
		mov cx, 0
		mov bx, [best_video_mode.width]
		cmp bx, [mode_info_block.width]
		jl save_and_continue
		je compare_height
		jmp next_mode

	compare_height:
		mov bx, [best_video_mode.height]
		cmp bx, [mode_info_block.height]
		jl save_and_continue
		je compare_bpp
		jmp next_mode

	compare_bpp:
		mov dx, [best_video_mode.bpp]
		mov ax, [mode_info_block.bpp]
		and ax, 11111111b
		cmp dx, ax
		jl save_and_continue
		jmp next_mode

	save_and_continue:
		mov bx, [mode_info_block.width]
		mov [best_video_mode.width], bx

		shr bx,3
		dec bx
		mov [best_video_mode.x_cur_max], bx

		mov bx, [mode_info_block.height]
		mov [best_video_mode.height], bx

		shr bx, 4
		dec bx
		mov word [best_video_mode.y_cur_max], ax

		mov ebx, 0
		mov bl, [mode_info_block.bpp]
		mov byte [best_video_mode.bpp], bl
		shr ebx, 3
		mov dword [best_video_mode.bytes_per_pixel], ebx


		mov bx, [mode]
		mov [best_video_mode.mode], bx
		mov ebx, [mode_info_block.framebuffer]
		mov dword[best_video_mode.framebuffer], ebx
		mov bx, [mode_info_block.pitch]
		mov word [best_video_mode.bytes_per_line],bx
		jmp next_mode

pop es

jmp $

new_line_and_next_mode:
	mov cx,1
	push cx
	jmp next_mode

reset_counter:
	mov cx, 1
	push cx
	jmp check_mode

set_mode:
	push es
	mov ax, 0x4F02
	mov bx, [best_video_mode.mode]
	or bx, 0x4000
	mov di, 0
	int 0x10
	pop es

	cmp ax, 0x4F
	clc

next_mode:
	mov ax, [segments]
	mov fs, ax
	mov si, [offset]
	jmp find_mode
