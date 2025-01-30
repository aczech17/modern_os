; load DH sectors to ES:BX from drive DL
disk_load:
    push dx

    mov ah, 0x02    ; BIOS read sector function
    mov al, dh      ; read DH sectors
    mov ch, 0x00    ; select cylinder 0
    mov dh, 0x00    ; select head
    mov cl, 0x02    ; start reading from second sector

    int 0x13

    jc disk_error

    pop dx
    cmp dh, al      ; if AL (read sectors) != DH (sectors expected)
    jne disk_error
    
    ret


disk_error:
    mov bx, DISK_ERROR_MSG
    call print_string
    jmp $

DISK_ERROR_MSG: db "Disk read error.", 0
