[org 0x0000]
    
    cli	; disable the interrupts because 16-bit interrupt vector will be invalid in 32 bit
	lgdt [gdt_descriptor]

	; set 0 bit in the CR0
	mov eax, cr0
	or eax, 0x01
	mov cr0, eax
jmp $
	jmp CODE_SEG:start_protected_mode

	[bits 32]
start_protected_mode:
	; set up the segment registers
	mov ax, DATA_SEG
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	; set up the stack
	mov ebp, 0x90000
	mov esp, ebp

	call clear_vga
	mov ebx, hello_message
	call print_string_vga

    jmp $

%include "src/print_string_vga.asm"

gdt_start:

gdt_null:
	dd 0
	dd 0
gdt_code:
	dw 0xFFFF 			; limit (bits 0-15)
	dw 0x0000 			; base (bits 0-15)
	db 0x0				; base (bits 16-23)
	db 1_00_1_1010b 	; 1 st flags, type flags
	db 1_1_0_0_1111b	; 2 nd flags, Limit (bits 16-19)
	db 0x0				; Base (bits 24-31)

gdt_data:
	dw 0xFFFF 			; limit (bits 0-15)
	dw 0x0000 			; base (bits 0-15)
	db 0x0				; base (bits 16-23)
	db 1_00_1_0010b 	; 1 st flags, type flags
	db 1_1_0_0_1111b	; 2 nd flags, Limit (bits 16-19)
	db 0x0				; Base (bits 24-31)
gdt_end:

gdt_descriptor:
	dw gdt_end - gdt_start - 1 ; GDT size - always less one of the true size
	dd gdt_start

; constants for indexing segments int the GDT
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start


hello_message db "Switched to 32 bit.", 0
