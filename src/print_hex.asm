print_hex:
	; number in BX
	pusha
	mov ah, 0x0E
	mov cl, 4
.loop:
	rol bx, 4
	mov al, bl
	and al, 0x0F
	
	add al, '0'
	cmp al, '9'
	jle .call_print
	
	add al, 'A' - ('0' + 10) ; correction for A-F
.call_print:
	int 0x10
	
	dec cl
	jnz .loop
	
	popa
	ret
