[bits 64]

VIDEO_MEMORY equ 0xB8000
VIDEO_MEMORY_END equ 0xB8FFF
COLOR equ 0x07

; RBX - string pointer
print_string_vga:
    mov rdx, VIDEO_MEMORY
    mov ah, COLOR
.loop:
    mov al, [rbx]

    cmp al, 0
    je .done

    mov [rdx], ax

    add rbx, 1
    add rdx, 2
    jmp .loop

.done:
    ret

[bits 32]
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
    