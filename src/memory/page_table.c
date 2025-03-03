#include "page_table.h"
#include "common.h"
#include <byteswap.h>
#include "../vga.h"
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

// static Page_table_entry entry_from_value(u64 value)
// {
//     value = bswap_64(value);

// }

static u64 frame_start_of_addr(u64 addr)
{
    return addr & ~(FRAME_SIZE - 1);
}

static void identity_map_page(Page_table_tree* pt_tree, u64 page_addr)
{
    for (u32 level = 0; level <= 3; ++level)
    {
        u32 shift = 39 - 9 * level;
        u32 index = (page_addr >> shift) & 0b111111111; // 9-bit index

        u64* table_entry_value_addr = &pt_tree->tables[level].entry[index];
        
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
        */

        u64 phys_addr;
        if (level < 3)
        {
            Page_table* next_level_table_addr = &pt_tree->tables[level + 1];
            phys_addr = (u64)next_level_table_addr;

            // Check if table address is page aligned.
            if (phys_addr & 0xFFF)
                panic("Table is not page aligned");
        }
        else
        {
            phys_addr = frame_start_of_addr(page_addr);
        }
        
        Page_table_entry entry =
        {
            .present = 1,
            .writable = 1,              // ???
            .user_accessible = 0,
            .write_through_caching = 0, // ???
            .cache_disable = 0,
            .accessed = 1,
            .dirty = 1, // ???
            .huge_page = 0,
            .global = 1,
            .available = 0,
            .phys_addr = phys_addr,
            .no_execute = 0,            // ???
        };
        
        *table_entry_value_addr = value(&entry);
    }
}

void zero_page_table_tree(Page_table_tree* tree)
{
    for (u32 level = 0; level < 4; ++level)
    {
        for (u32 entry_num = 0; entry_num < 512; ++entry_num)
        {
            tree->tables[level].entry[entry_num] = 0;
        }
    }
}

void identity_map_kernel(Page_table_tree* pt_tree, const Memory_map* kernel_regions)
{
    for (u64 region = 0; region < kernel_regions->region_count; ++region)
    {
        u64 region_start = kernel_regions->start_addr[region];
        u64 region_end = kernel_regions->end_addr[region];

        for (u64 page_addr = frame_start_of_addr(region_start); page_addr < region_end; page_addr += FRAME_SIZE)
        {
            identity_map_page(pt_tree, page_addr);
        }
    }

    // Identity map VGA buffer pages as well.
    const u64 vga_start = 0xB8000;
    const u64 vga_end = vga_start + VGA_SIZE - 1;
    for (u64 page_addr = frame_start_of_addr(0xB8000); page_addr < vga_end; page_addr += FRAME_SIZE)
    {
        identity_map_page(pt_tree, page_addr);
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
        u64 table_entry_value = tree->tables[level].entry[index];

        // Check if present.
        if (!(table_entry_value & 1))
            return INVALID_ADDR;

        table_addr = table_entry_value & 0x000FFFFFFFFFF000;        
    }
    u64 page_addr = table_addr;
    u64 page_offset = virt_addr & 0xFFF;

    return page_addr | page_offset;
}
