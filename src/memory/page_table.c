#include "page_table.h"
#include "common.h"
#include <byteswap.h>

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

static u64 value(const Page_table_entry* entry)
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
    return bswap_64(val);
}

static void identity_map_region(Page_table_tree* pt_tree, u64 region_start, u64 region_end)
{
}

void identity_map_kernel(Page_table_tree* pt_tree, const Memory_map* kernel_regions)
{
    for (u64 region = 0; region < kernel_regions->region_count; ++region)
    {
        identity_map_region(pt_tree, kernel_regions->start_addr[region], kernel_regions->end_addr[region]);
    }
}

u64 get_phys_addr(const Page_table_tree* tree, u64 virt_addr)
{
    // level 1 -> 0 -> 39
    // level 2 -> 1 -> 30
    // level 3 -> 2 -> 21
    // level 4 -> 3 -> 12
    // page offset  -> 0

    u64 table_addr;
    for (u32 level = 0; level <= 3; ++level)
    {
        u32 shift = 39 - 9 * level;
        u32 index = (virt_addr >> shift) & 0b111111111; // 9-bit index
        table_addr = tree->tables[level].entry[index];

        // Check if present.
        if (!(table_addr & 1))
            return INVALID_ADDR;
    }
    u64 page_addr = table_addr & 0x000FFFFFFFFFF000;
    u64 page_offset = virt_addr & 0xFFF;

    return page_addr | page_offset;
}
