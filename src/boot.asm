%include "out/sectors.inc" ; count of sectors of the stage2

[org 0x7C00]
jmp word 0x0000:start ; ???

start:
	; Initialize the stack.
	;mov bp, 0x8000
	;mov sp, bp

	; Now try to read the 2nd sector of the disk.

	; read the sector to 0x2000:0x0000
	mov ax, 0x2000
	mov es, ax
	mov bx, 0

	; drive number in DL already set by BIOS
	mov ah, 2	; 2 read sector BIOS routine
	mov al, SECTORS_TO_READ
	mov ch, 0	; cylinder 0
	mov cl, 2	; sector 2 (they start from 1)
	mov dh, 0	; head number 0 (they start from 0(wtf?))
	
	int 13h
	jc error
	

	jmp word 0x2000:0x0000

	; The CPU should never reach here.

error:
	mov bx, error_message
	mov ah, 0x0E
.loop:
	mov al, [bx]
	cmp al, 0
	je .halt

	int 0x10
	inc bx
	jmp .loop
.halt:
	jmp $


error_message: db "Error reading disk.", 0

times 510 - ($ - $$) db 0
db 0x55
db 0xAA
