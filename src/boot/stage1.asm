%include "out/sectors.inc" ; count of sectors of compiled stage2 and kernel

[org 0x7C00]
jmp word 0x0000:start ; Make sure we have address 0x0000:0x7C00, NOT 0x07C0:0x0000.

start:
	; Read the 2nd stage of the bootloader from the 2nd sector of the disk.
	mov ah, 0x42	; extended read
					; DL already set (?)
	mov si, disk_address_packet
	int 0x13

	jc error
	

	jmp word 0x2000:0x0000

	; The CPU should never reach here.
	jmp 0xFFFF:0x0000 ; Force reboot if it reaches here after all.




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

; Disk address packet is a structure for BIOS to read linear sectors.
disk_address_packet:
	db 0x10		; structure size
	db 0		; reserved

	dw SECTORS_TO_READ

	; read to [0x2000:0]
	dw 0		; offset
	dw 0x2000	; segment

	dq 1		; sector 1

times 446 - ($ - $$) db 0
partition_table:
    db  0x80  ; bootable
    db  0x00  ; Head start
    db  0x01  ; Cylinder/Sector start
    db  0x00  ; Cylinder start (MSB)
    db  0x0C  ; FAT32 LBA
    db  0xFE  ; Head end
    db  0xFF  ; Cylinder/Sector end
    db  0xFF  ; Cylinder end (MSB)
    dd  0x00000001  ; LBA start partition (first sector after MBR)
    dd  0xFFFFFFFF  ; Size of partition in sectors (maximum).

    times 48 db 0  ; Null partitions (3 partitions 16B each)

times 510 - ($ - $$) db 0
db 0x55
db 0xAA
