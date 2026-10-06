#include "page_table.h"
#include "common.h"
#include "../common.h"

/*
    u8 present;
    u8 writable;
    u8 user_accessible;
    u8 write_through_caching;
    u8 cache_disable;
    u8 accessed;
    u8 dirty;
    u8 huge_page;
    u8 global;
    u64 available;
    u64 phys_addr;
    u64 no_execute;


    0	present	the page is currently in memory
    1	writable	it’s allowed to write to this page
    2	user accessible	if not set, only kernel mode code can access this page
    3	write through caching	writes go directly to memory
    4	disable cache	no cache is used for this page
    5	accessed	the CPU sets this bit when this page is used
    6	dirty	the CPU sets this bit when a write to this page occurs
    7	huge page/null	must be 0 in P1 and P4, creates a 1GiB page in P3, creates a 2MiB page in P2
    8	global	page isn’t flushed from caches on address space switch (PGE bit of CR4 register must be set)
    9-11	available	can be used freely by the OS
    12-51	physical address	the page aligned 52bit physical address of the frame or the next page table. So it's 40 bits.
    52-62	available	can be used freely by the OS
    63	no execute	forbid executing code on this page (the NXE bit in the EFER register must be set)
*/

u64 page_table_value(const Page_table_entry* entry)
{
    u64 val =
        (entry->present << 0) |
        (entry->writable << 1) |
        (entry->user_accessible << 2) |
        (entry->write_through_caching << 3) |
        (entry->cache_disable << 4) |
        (entry->accessed << 5) |
        (entry->dirty << 6) |
        (entry->huge_page << 7) |
        (entry->global << 8) |
        ((entry->available & 0b111) << 9) |
        entry->phys_addr |      // It should be page aligned, so that no shift is needed.
        ((entry->available >> 3) << 52) |
        (entry->no_execute << 63);

    return val;
}

Phys_addr get_phys_addr(const Page_table* pt_root, Virt_addr virt_addr)
{
    Page_table* pt = (Page_table*)pt_root;
    Phys_addr phys_addr, frame_base, frame_offset;

    for (u32 level = 1; level <= 4; ++level)
    {
        u64 virt_addr_shift = 48 - 9 * level;
        u64 index = (virt_addr >> virt_addr_shift) & 0b111111111;

        u64 entry_value = pt->entry[index];

        if (!(entry_value & PTE_PRESENT))
            return INVALID_ADDR;

        phys_addr = entry_value & PTE_PHYSICAL_ADDRESS_MASK;

        if ((level == 2 || level == 3) && (entry_value & PTE_HUGE_PAGE))
        {
            frame_base = phys_addr;

            if (level == 2)
                frame_offset = virt_addr & 0x3FFFFFFF;  // 30 lowest bits
            else // level == 3
                frame_offset = virt_addr & 0x1FFFFF;    // 21 lowest bits

            return frame_base | frame_offset;
        }

        pt = (Page_table*)phys_addr;
    }

    frame_base = phys_addr;
    frame_offset = virt_addr & 0xFFF;

    return frame_base | frame_offset;
}
