print_string:
	; BX - string
	pusha
	mov ah, 0x0E

.loop:
	mov al, [bx]
	cmp al, 0
	je .loop_end
	int 0x10
	
	inc bx
	jmp .loop
.loop_end:
	popa
	ret
