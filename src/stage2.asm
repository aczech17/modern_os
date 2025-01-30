[bits 16]
[org 0x0000]
    
    cli	; disable the interrupts because 16-bit interrupt vector will be invalid in 32 bit

	; set up segment registers
	mov ax, 0x2000
	mov ds, ax
	mov es, ax


	lgdt [gdt_32_addr]

	; set 0 bit in the CR0
	mov eax, cr0
	or eax, 0x01
	mov cr0, eax

	jmp dword 0x8:(0x20000+start_protected_mode)

	[bits 32]
start_protected_mode:

	; set up the segment registers
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax

	; set up the stack
	mov ebp, 0x90000
	mov esp, ebp

	call clear_vga
	mov ebx, hello_message
	call print_string_vga

    jmp $

%include "src/print_string_vga.asm"

gdt_32_addr:
	dw (gdt_32.end - gdt_32) - 1
	dd 0x20000 + gdt_32

;times (32 - ($ - $$) % 32) db 0xcc

gdt_32:
.null:
	dd 0
	dd 0
.code:
	dd 0xffff  ; segment limit
	dd (10 << 8) | (1 << 12) | (1 << 15) | (0xf << 16) | (1 << 22) | (1 << 23)

.data:
	dd 0xffff  ; segment limit
	dd (2 << 8) | (1 << 12) | (1 << 15) | (0xf << 16) | (1 << 22) | (1 << 23)
.null_2:
	dd 0
	dd 0
.end:


hello_message db 'witched to 32 bit.', 0

;times (512 - ($ - $$) % 512) db 0
