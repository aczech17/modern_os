#include "page_table.h"
#include "common.h"
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

    return val;
}

static Phys_addr frame_start_of_addr(u64 addr)
{
    return addr & ~(FRAME_SIZE - 1);
}

static Phys_addr get_pte_addr_from_page(const Page_table* pt_root, Virt_addr page_address)
{
    // level 1 -> 0 -> 39
    // level 2 -> 1 -> 30
    // level 3 -> 2 -> 21
    // level 4 -> 3 -> 12
    // page offset  -> 0

    Page_table* pt = (Page_table*)pt_root;
    Phys_addr phys_addr;
    for (u32 level = 0; level <= 2; ++level)
    {
        u64 virt_addr_shift = 39 - 9 * level;
        u64 index = (page_address >> virt_addr_shift) & 0b111111111; // 9-bit index

        u64 pte_value = pt->entry[index];

        if (!(pte_value & PTE_PRESENT))
            return INVALID_ADDR;

        phys_addr = pte_value & PTE_PHYSICAL_ADDRESS_MASK;
        pt = (Page_table*) phys_addr;
    }

    return phys_addr;
}

Phys_addr get_phys_addr(const Page_table* pt_root, Virt_addr virt_addr)
{
    Virt_addr page_address = virt_addr & ~(0xFFF);
    Phys_addr pte_addr = get_pte_addr_from_page(pt_root, page_address);
    u64 pte = *(u64*)pte_addr;

    Phys_addr frame_address = pte & PTE_PHYSICAL_ADDRESS_MASK;
    Phys_addr page_offset = virt_addr & 0xFFF;

    return frame_address | page_offset;
}

static void identity_map_page(Page_table* pt_tree, Virt_addr page_addr)
{
    Page_table_entry entry =
    {
        .present = 1,
        .writable = 1,              // ???
        .user_accessible = 0,
        .write_through_caching = 0, // ???
        .cache_disable = 0,
        .accessed = 0,
        .dirty = 0,
        .huge_page = 0,
        .global = 1,
        .available = 0,
        .phys_addr = page_addr,     // identity mapping
        .no_execute = 0,            // ???
    };
    u64 pte_value = value(&entry);

    u64* pte_addr = (u64*)get_pte_addr_from_page((const Page_table*)pt_tree, page_addr);
    *pte_addr = pte_value;
}

void identity_map_kernel(Page_table* pt_tree, const Phys_memory_map* kernel_regions)
{
    for (u64 region = 0; region < kernel_regions->region_count; ++region)
    {
        Phys_addr region_start = kernel_regions->start_addr[region];
        Phys_addr region_end = kernel_regions->end_addr[region];

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
