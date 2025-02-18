#include "page_table.h"

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
    12-51	physical address	the page aligned 52bit physical address of the frame or the next page table
    52-62	available	can be used freely by the OS
    63	no execute	forbid executing code on this page (the NXE bit in the EFER register must be set)
*/

u64 value(Page_table_entry* entry)
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

    // Flip bytes because little endian.
    u64 flipped_value = 0;
    for (int b = 0; b < 8; ++b)
    {
        flipped_value <<= 8;
        flipped_value |= (val >> (b * 8)) & 0xFF;
    }

    return flipped_value;
}
