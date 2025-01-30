[bits 32]

VIDEO_MEMORY equ 0xB8000
VIDEO_MEMORY_END equ 0xB8FFF
COLOR equ 0x07

; EBX - string pointer
print_string_vga:
    pusha
    mov edx, VIDEO_MEMORY
    mov ah, COLOR
.loop:
    mov al, [ebx]

    cmp al, 0
    je .done

    mov [edx], ax

    add ebx, 1
    add edx, 2
    jmp .loop

.done:
    popa
    ret

clear_vga:
    pusha
    mov edx, VIDEO_MEMORY
    mov ah, COLOR
    mov al, ' '
.loop:
    mov [edx], ax
    add edx, 2
    cmp edx, VIDEO_MEMORY_END
    jle .loop

    popa
    ret
    