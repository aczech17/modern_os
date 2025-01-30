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
	; Set up the segment registers.
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax

	; Set up the stack.
	mov ebp, stack_top
	mov esp, ebp


	call set_up_page_tables
	call enable_paging

	lgdt [gdt_64_addr + 0x20000]
	jmp dword 0x8:(0x20000 + start_64)

	[bits 64]
start_64:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax

	mov rbp, stack_top
	mov rsp, rbp

	call clear_vga
	mov rbx, hello_message + 0x20000
	call print_string_vga
    jmp $


CODE_END:


%include "src/print_string_vga.asm"

[bits 32]
set_up_page_tables:
	mov eax, page_table.level3
	or eax, 0b11 ; present + writable
	mov [page_table.level4], eax

	mov eax, page_table.level2
	or eax, 0b11 ; present + writable
	mov [page_table.level3], eax

	; Map each level2 entry to a huge 2MiB page.
	mov ecx, 0
.map_p2_table:
	; Map ecx-th P2 entry to a huge page that starts at address 2MiB * ECX.
	mov eax, 1 << 21 ; 2MiB
	mul ecx ; start address of ECX-th page
	or eax, 0b10000011 ; huge + present + writable
	mov [page_table.level2 + ecx * 8], eax ; map ECX-th entry

	inc ecx
	cmp ecx, 512
	jne .map_p2_table

	ret

enable_paging:
	; Load level4 table to CR3 register.
	mov eax, page_table.level4
	mov cr3, eax

	; Enable PAE flag in CR4 (physical address extension).
	mov eax, cr4
	or eax, 1 << 5
	mov cr4, eax

	; Set long mode bit in a model specific register called EFER
	mov ecx, 0xC0000080 ; EFER number
	rdmsr
	or eax, 1 << 8 	; EFER content lands in EAX so modify it
	wrmsr			; and write it back.

	; Enable paging in the CR0 register.
	mov eax, cr0
	or eax, 1 << 31
	mov cr0, eax

	ret


gdt_32_addr:
	dw (gdt_32.end - gdt_32) - 1
	dd 0x20000 + gdt_32
times (32 - ($ - $$) % 32) db 0xCC

gdt_32:
	; Flat mode, base = 0x0, limit = 0xFFFFFFFF
.null:
	dd 0
	dd 0
.code:
	dd 0x0000_FFFF  ; base (15 - 0) limit (15 - 0)
	dd    (0b1010 << 8) 					    | (1 << 12) 		|   (1 << 15) | (0xF << 16)   | (1 << 22)     | (1 << 23)
	;  code nonconforming readable notaccessed, descriptor type=code   present    limit(19-16)   32 bit segment  granularity
.data:
	dd 0xFFFF  ; segment limit
	dd (2 << 8) | (1 << 12) | (1 << 15) | (0xF << 16) | (1 << 22) | (1 << 23)
.null_2:
	dd 0
	dd 0
.end:

gdt_64_addr:
	dw (gdt_64.end - gdt_64) - 1
	dd 0x20000 + gdt_64
times (32 - ($ - $$) % 32) db 0xCC

gdt_64:
	; Flat mode again
.null:
	dd 0
	dd 0
.code:
	; Same as 32 but clear D/B and set L (64-bit code segment)
	dd 0xFFFF  ; segment limit
	dd (10 << 8) | (1 << 12) | (1 << 15) | (0xF << 16) | (1 << 21) | (1 << 23)
.data:
	dd 0xFFFF  ; segment limit
	dd (2 << 8) | (1 << 12) | (1 << 15) | (0xF << 16) | (1 << 21) | (1 << 23)
.null2:
	dd 0
	dd 0
.end:
;times (4096 - ($ - $$) % 4096) db 0


section .bss
align 4096

page_table:
.level4:
	resb 4096
.level3:
	resb 4096
.level2:
	resb 4096

stack_bottom:
	resb 16 * 1024
stack_top:

section .rodata
hello_message db 'Booting...', 0

;times (512 - ($ - $$) % 512) db 0
