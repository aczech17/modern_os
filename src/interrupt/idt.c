#include "idt.h"
#include "handlers.h"
#include <stddef.h>

static void setup_idt_entry(Idt_entry idt_entries[256], const size_t entry_num, const u64 isr_address)
{
	idt_entries[entry_num].offset_low		= isr_address & 0xFFFF;
	idt_entries[entry_num].offset_middle	= (isr_address >> 16) & 0xFFFF;
	idt_entries[entry_num].offset_high		= (isr_address >> 32) & 0xFFFFFFFF;

	idt_entries[entry_num].segment_selector = 0x08;
	idt_entries[entry_num].ist = 0;

	/*
	IDT attributes:
	bit:   7   6 5   4 3 2 1 0
		   ─   ─ ─     ─ ─ ─ ─
		   P   DPL      TYPE
		Present = 1
		DPL     = 00
		TYPE    = 1110 (64-bit interrupt gate)

		0b10001110 = 0x8E
	 */

	idt_entries[entry_num].type_attrs = 0x8E;
	idt_entries[entry_num].reserved = 0;
}

void setup_idt(Idt_entry idt_entries[256])
{
	// 14
	setup_idt_entry(idt_entries, 14, (u64)page_fault_isr);

	u8 idtr[10];

	u16 limit = 256 * sizeof(Idt_entry) - 1;
	u64 base = (u64)idt_entries;

	idtr[0] = limit;
	idtr[1] = limit >> 8;

	for (u8 i = 0; i < 8; ++i)
		idtr[i + 2] = base >> (8 * i);

	__asm__ volatile
	(
		"lidt %0"
		:
		: "m"(idtr)
	);
}
