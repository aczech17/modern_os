SMAP 						equ 0x0534D4150
MAX_MEMORY_SECTIONS_COUNT	equ 128

STAGE2_SECTORS 				equ (stage2_end - stage2) / 512
KERNEL_LBA 					equ (STAGE2_SECTORS + 1)
KERNEL_BUFFER_SEGMENT		equ 0x2000
KERNEL_BUFFER_OFFSET 		equ 0x4000
KERNEL_BUFFER_SECTORS 		equ 64


%include "out/mem_layout.inc"
; After include we've got:
;	KERNEL_BLOB_SIZE
;	KERNEL_BLOB_SECTORS
;	STACK_BOTTOM
;	STACK_SIZE
;	STACK_TOP
;	KERNEL_TEXT_ADDR


KERNEL_BLOB_ADDRESS equ (KERNEL_TEXT_ADDR + KERNEL_BLOB_SIZE)

[bits 16]
[org 0x20000]
stage2:
; set up segment registers
	mov ax, 0x2000
	mov ds, ax
	mov es, ax

	mov ax, 0x1f00
	mov ss, ax
	mov sp, 0
	
	; Save boot drive.
	mov [boot_drive], dl
enable_a20:
	; http://wiki.osdev.org/A20_Line
	mov ax, 0x2401
	int 0x15

	in al, 0x92
	or al, 2
	out 0x92, al


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


load_kernel:
.next:
	mov ebx, [kernel_sectors_left]

	cmp ebx, 0
	je .done

	cmp ebx, KERNEL_BUFFER_SECTORS
	jle .fits
.trim:
	mov ebx, KERNEL_BUFFER_SECTORS
.fits:
	mov [dap.sector_count], bx

	mov ah, 0x42
	mov si, dap
	mov dl, [boot_drive]

	int 0x13
	jc .error

	; Temporarily enter 32 bit mode.
	cli
	lgdt [gdt_32.addr]

	mov eax, cr0
	or eax, 1
	mov cr0, eax

	jmp dword 0x8:.enter_32
[bits 32]
.enter_32:
	mov ax, 0x10
    mov ds, ax
    mov es, ax
	mov fs, ax
    mov gs, ax

	; REP MOVSB
	; DS:SI -> ES:DI

	; ECX is number of bytes to copy.
	; dap.sector_counts has info about 
	; how many sectors were read to memory.
	; We want to copy sectors_count * 512 bytes.
	movzx ecx, word [dap.sector_count]
	shl ecx, 9

	; Set destination in EDI.
	mov edi, [kernel_destination]

	; We conform to real mode address calculation:
	; Segment << 4 + offset
	mov esi, KERNEL_BUFFER_SEGMENT
	shl esi, 4
	add esi, KERNEL_BUFFER_OFFSET
	
	rep movsb


	; Return to real mode.
	jmp 0x18:.protected16
    [bits 16]
.protected16:
	mov eax, cr0
	and eax, ~1
	mov cr0, eax

	jmp 0x2000:.real16
.real16:
	mov ax, 0x2000
	mov ds, ax
	mov es, ax
	
	movzx eax, word [dap.sector_count]

	add dword [dap.lba], eax
	adc dword [dap.lba + 4], 0

	sub dword [kernel_sectors_left], eax

	
	shl eax, 9
	; Now EAX has number of bytes read.
	add dword [kernel_destination], eax
	
	jmp .next
.error:
	mov ax, 0xB800
	mov es, ax
	mov word [es:0], 0x0C58
	jmp $
.done:

	cli		; Disable interrupts once again, just to be sure.
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

parse_kernel:
	mov rsi, [abs KERNEL_BLOB_ADDRESS + 0x20]		; Load e_phof - start of the program header table.
	add rsi, KERNEL_BLOB_ADDRESS						; Now in RSI we have the memory address of e_phof.

	movzx ecx, word [abs KERNEL_BLOB_ADDRESS + 0x38]	; Load e_phnum -- the number of entries in the program header table.

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
	lea rsi, [abs KERNEL_BLOB_ADDRESS + r8d]	; source (kernel start + p_offset)
	mov rdi, r9							; destination  (p_vaddr)
	mov rcx, r10					; count of bytes (p_filesz)
	rep movsb

	; Restore registers (RCX and RSI)
	mov rcx, r15
	mov rsi, rbp	

.next:
	add rsi, 0x38	; 0x38 -- program header size
	dec rcx
	jnz .ph_loop


set_up_stack:
	mov rbp, STACK_TOP
	mov rsp, rbp

set_kernel_arguments:
	; Calling convention:
	; rdi, rsi, rdx, rcx, r8, r9

	mov rdi, memory_sections.entries
	mov rsi, 0
	mov esi, dword [abs memory_sections.count]

	; Program header table
	mov rdx, [abs KERNEL_BLOB_ADDRESS + 0x20]
	add rdx, KERNEL_BLOB_ADDRESS

	; Program header entry count
	movzx rcx, word [abs KERNEL_BLOB_ADDRESS + 0x38]	; e_phnum -- numer of ph entries.

	mov r8, STACK_SIZE
	
jump_to_kernel:
	mov rax, [abs KERNEL_BLOB_ADDRESS + 0x18]		; e_entry -- entry point
	call rax									; Finally jump to the kernel.

	; The CPU should never reach here.
	jmp $



boot_drive:				db 0
kernel_sectors_left:	dd KERNEL_BLOB_SECTORS
kernel_destination:		dd KERNEL_BLOB_ADDRESS

align 4
dap:
	db 0x10	; size
	db 0	; reserved
.sector_count:
	dw KERNEL_BUFFER_SECTORS 	; int 13 resets this to # of 
								; sectors actually read/written
.offset:
	dw KERNEL_BUFFER_OFFSET
.segment:
	dw KERNEL_BUFFER_SEGMENT
.lba:
	dq KERNEL_LBA

memory_sections:
.count:
	times 4 db 0
.entries:
	times MAX_MEMORY_SECTIONS_COUNT * 24 db 0

align 32
gdt_32:
    dq 0                        ; 0x00 - null descriptor

.code1:
    db 0xFF						; limit[7:0]
	db 0xFF						; limit[15:8]
	
    db 0						; base[7:0]
	db 0						; base[15:8]
    db 0						; base[23:16]

    db 0x9A                     ; code, present, readable

    db 0xCF                     ; limit[19:16] + flags
	; 4KB gran, 32-bit
	

    db 0						; base [31:24]

.data:
    db 0xFF
	db 0xFF


    db 0
	db 0
    db 0

    db 0x92                     ; data, present, writable

    db 0xCF                     ; 4KB gran, 32-bit
	
    db 0

.code2:
    dw 0xFFFF
    dw 0
    db 0
    db 0x9A                     ; code, present, readable
    db 0x0F                     ; 16-bit, limit 64KB
    db 0
.addr:
    dw gdt_32.addr - gdt_32 - 1
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

times (512 - (($ - $$) % 512)) % 512 db 0 ; Padding to full sector.
stage2_end:

