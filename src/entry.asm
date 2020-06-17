bits 16

org 0x7c00

jmp start

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

start:
	mov ax, 0x003
	int 0x10

	mov si, hello
	call print
	jmp $

hello db "Loading Melvix...", 0x0A, 0x0D, 0x00

times 510 - ($ - $$) db 0
dw 0xAA55
