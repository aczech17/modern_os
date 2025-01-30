[bits 16]
[org 0x0000]
    
	; Enable A20 http://wiki.osdev.org/A20_Line
	mov ax, 0x2401
	int 0x15

	in al, 0x92
	or al, 2
	out 0x92, al

	; Disable the interrupts because 16-bit interrupt vector will be invalid in 32 bit.
    cli

	; set up segment registers
	mov ax, 0x2000
	mov ds, ax
	mov es, ax

	mov ax, 0x1f00
	mov ss, ax
	mov sp, 0

	lgdt [gdt_32_addr]

	; Set 0 bit in the CR0 to enable protected mode.
	mov eax, cr0
	or eax, 0x01
	mov cr0, eax

	; Far jump to clean the pipeline and finally start the 32 bit mode.
	jmp dword 0x8:(0x20000+start_32)

	[bits 32]
start_32:

	; set up the segment registers
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax

	; set up the stack
	;mov ebp, 0x90000
	;mov esp, ebp

	mov ebp, stack_top
	mov esp, ebp

	;call clear_vga
	;mov ebx, hello_message
	;call print_string_vga


    jmp $


CODE_END:

;%include "src/print_string_vga.asm"

gdt_32_addr:
	dw (gdt_32.end - gdt_32) - 1
	dd 0x20000 + gdt_32

times (32 - ($ - $$) % 32) db 0xcc

gdt_32:
	; Flat mode, base = 0x0, limit = 0xFFFFFFFF
.null:
	dd 0
	dd 0
.code:
	dd 0x0000_FFFF  ; base (15 - 0) limit (15 - 0)
	dd    (0b1010 << 8) 					    | (1 << 12) 		|   (1 << 15) | (0xf << 16)   | (1 << 22)     | (1 << 23)
	
	;  code nonconforming readable notaccessed, descriptor type=code   present    limit(19-16)   32 bit segment  granularity
	

.data:
	dd 0xffff  ; segment limit
	dd (2 << 8) | (1 << 12) | (1 << 15) | (0xf << 16) | (1 << 22) | (1 << 23)
.null_2:
	dd 0
	dd 0
.end:


hello_message db 'witched to 32 bit.', 0

section .bss
stack_bottom:
	resb 4 * 1024
stack_top:

;times (512 - ($ - $$) % 512) db 0
