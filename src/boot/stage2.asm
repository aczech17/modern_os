SMAP equ 0x0534D4150
MAX_MEMORY_SECTIONS_COUNT equ 128
%include "out/stack_size.inc"

[bits 16]
[org 0x20000]
stage2:
	; Enable A20 http://wiki.osdev.org/A20_Line
	mov ax, 0x2401
	int 0x15

	in al, 0x92
	or al, 2
	out 0x92, al

; set up segment registers
	mov ax, 0x2000
	mov ds, ax
	mov es, ax

	mov ax, 0x1f00
	mov ss, ax
	mov sp, 0

detect_memory:
	mov di, memory_sections.entries
	mov ebx, 0
	mov bp, 0	; BP -- entry count
	mov edx, SMAP
	mov eax, 0xE820
	mov [di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24		; ask for 24 bytes
	int 0x15
	jc .fail

	mov edx, SMAP ; Some BIOSES trash this register.
	cmp eax, edx  ; On success, EAX is set to SMAP.
	jne .fail

	test ebx, ebx ; If EBX == 0, the list is only 1 entry long (worthless).
	je .fail

	jmp .jmpin

.e820lp:
	mov eax, 0xE820	; EAX is trashed every int 0x15 call.
	mov [di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24		; ask for 24 bytes again
	int 0x15

	jc .e820f		; If carry is set, then the end is reached.
	mov edx, SMAP	; Trashing may occur again.
.jmpin:
	jcxz .skipent
	cmp cl, 20 	; got a 24 byte ACPI 3.X response?
	jbe .notext
	test byte [di + 20], 1	; if so: is the "ignore this data" bit clear?
	je .skipent
.notext:
	mov ecx, [di + 8]	; get lower u32 of memory region length
	or ecx, [di + 12]	; "or" it with upper u32 to test for zero
	jz .skipent				; if u64 length is zero, skip entry
	inc bp					; got a good entry, ++count, move to the next storage spot
	add di, 24
.skipent:
	test ebx, ebx ; if EBX resets to 0, list is complete
	jne .e820lp
.e820f:
	mov [memory_sections.count], bp ; store the entry count
	clc	; clear carry flag
	jmp .done
.fail:
	jmp $
.done:

    cli ; Disable the interrupts because 16-bit interrupt vector will be invalid in 32 bit.
	lgdt [gdt_32.addr]

	; Set 0 bit in the CR0 to enable protected mode.
	mov eax, cr0
	or eax, 0x01
	mov cr0, eax

	; Far jump to clean the pipeline and finally start the 32 bit mode.
	jmp dword 0x8:start_32

	[bits 32]
start_32:
	; Set up the segment registers.
	;
	;	15         3  2  1  0
	;	+------------+--+--+--+
	;	| GDT index  |TI| CPL |
	;	+------------+--+--+--+

	; Data segments
	mov ax, (2 << 3)
	mov ds, ax
	mov es, ax
	mov ss, ax

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

	lgdt [gdt_64.addr]
	jmp dword 0x8:start_64

	[bits 64]
start_64:
	; Set up segment registers once again.
	;
	;	15         3  2  1  0
	;	+------------+--+--+--+
	;	| GDT index  |TI| CPL |
	;	+------------+--+--+--+

	mov ax, (2 << 3)
	mov ds, ax
	mov es, ax
	mov ss, ax

    
	; Now let's load the kernel.
load_kernel:
	mov rsi, [kernel + 0x20]	; Load e_phof -- start of the program header table.
								; Now in RSI we have the offset from the start of the kernel file,
								; but we want the offset in the memory, so
	add rsi, kernel				; we add to RSI the size of the stage 2.

	movzx ecx, word [kernel + 0x38]	; Load e_phnum -- the number of entries in the program header table.

	cld			; Clear direction flag for future rep movsb.
.ph_loop:
	mov eax, [rsi + 0]	; ph_type
	cmp eax, 1			; Check if PT_LOAD, if not then ignore.
	jne .next
	
	mov r8,  [rsi + 0x08] 	; p_offset
	mov r9,  [rsi + 0x10]	; p_vaddr -- address of the segment
	mov r10, [rsi + 0x20]	; p_filesz -- size of the segment in the kernel file
	mov r11, [rsi + 0x28]	; p_memsz  -- size of the segment in memory


	; Save registers (RSI and RCX)
	mov rbp, rsi
	mov r15, rcx

	; zero memory
	mov rdi, r9		; set destination address (p_vaddr)
	mov rcx, r11	; set segment size (p_memsz)
	mov al, 0		; value is 0
	rep stosb

	; load ELF section to memory
	lea rsi, [kernel + r8d]	; source (kernel start + p_offset)
	mov rdi, r9				; destination  (p_vaddr)
	mov rcx, r10			; count of bytes (p_filesz)
	rep movsb

	; Restore registers (RCX and RSI)
	mov rcx, r15
	mov rsi, rbp	

.next:
	add rsi, 0x38	; 0x38 -- program header size
	dec rcx
	jnz .ph_loop


set_up_stack:
	mov rbp, stack.bottom
	mov rsp, rbp

set_kernel_arguments:
	; Calling convention:
	; rdi, rsi, rdx, rcx, r8, r9

	mov rdi, memory_sections.entries
	mov rsi, 0
	mov esi, dword [memory_sections.count]

	; Program header table
	mov rdx, [kernel + 0x20]
	add rdx, kernel

	; Program header entry count
	movzx rcx, word [kernel + 0x38]	; e_phnum -- numer of ph entries.

	mov r8, STACK_SIZE
	
jump_to_kernel:
	mov rax, [kernel + 0x18]		; e_entry -- entry point
	call rax						; Finally jump to the kernel.

	jmp $


memory_sections:
.count:
	times 4 db 0
.entries:
	times MAX_MEMORY_SECTIONS_COUNT * 24 db 0

align 32
gdt_32:
	; Flat mode, base = 0x0, limit = 0xFFFFFFFF
.null:
	dd 0
	dd 0
.code:
	dd 0x0000_FFFF  ; base (15 - 0) limit (15 - 0)
	dd    (0b1010 << 8) 					    | (1 << 12) 		|  (1 << 15) | (0xF << 16)   | (1 << 22)     | (1 << 23)
	;  code nonconforming readable notaccessed, descriptor type=code   present     limit(19-16)    32 bit segment  granularity
.data:
	dd 0xFFFF  ; segment limit
	dd (2 << 8) | (1 << 12) | (1 << 15) | (0xF << 16) | (1 << 22) | (1 << 23)
.null_2:
	dd 0
	dd 0
.end:
.addr:
	dw (.end - gdt_32) - 1
	dd gdt_32


align 32
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
.addr:
	dw (.end - gdt_64) - 1
	dd gdt_64



align 4096
page_table:
.level4:
	times 512 dq 0
.level3:
	times 512 dq 0
.level2:
	times 512 dq 0

section .bss
stack:
.top:
	resb STACK_SIZE
.bottom:

section .text
times (512 - ($ - $$) % 512) db 0
kernel:
