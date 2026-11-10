#include "page_table.h"
#include "../common.h"
#include "common.h"

/*
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

/*
    lvl1    lvl2    lvl3    lvl4    offset
    47-39   38-30   29-21   20-12   11-0
*/

/// Identity mapping kernel.
void setup_page_tables
(
    Page_table* pt_level_1,
    Page_table* pt_level_2,
    Page_table* pt_level_3,
    Page_table pt_level_4[512],
    Phys_memory_map* kernel_regions
)
{
    for (int i = 0; i < 512; ++i)
    {
        pt_level_1->entry[i] = pt_level_2->entry[i] = pt_level_3->entry[i] = 0;

        // Also zero all the 512 level 4 tables.
        memset(&pt_level_4[i], 0, sizeof(Page_table));
    }

    // Identity map kernel regions.
    // Current algorithm is not very effective.
    for (u64 i = 0; i < kernel_regions->region_count; ++i)
    {
        Phys_addr start = kernel_regions->start_addr[i];
        Phys_addr end   = kernel_regions->end_addr[i];

        Phys_addr low_frame_addr = (start / FRAME_SIZE) * FRAME_SIZE;
        Phys_addr hi_frame_addr  = (end / FRAME_SIZE) * FRAME_SIZE;

        for (Phys_addr frame_addr = low_frame_addr; frame_addr <= hi_frame_addr; frame_addr += FRAME_SIZE)
        {
            u64 level_4_table_index = (frame_addr >> 21) & 0x1FF;
            u64 level_4_entry_index = (frame_addr >> 12) & 0x1FF;

            pt_level_4[level_4_table_index]
                .entry[level_4_entry_index] =
                PTE_PRESENT | PTE_WRITABLE | (frame_addr & PTE_4_KIB_PHYSICAL_ADDRESS_MASK);


            if (!(pt_level_3->entry[level_4_table_index] & PTE_PRESENT))
            {
                pt_level_3->entry[level_4_table_index] =
                    PTE_PRESENT | PTE_WRITABLE |
                        ((Phys_addr)(&pt_level_4[level_4_table_index]) & PTE_4_KIB_PHYSICAL_ADDRESS_MASK);
            }
        }
    }


    pt_level_2->entry[0] = PTE_PRESENT | PTE_WRITABLE | ((Phys_addr)pt_level_3 & PTE_4_KIB_PHYSICAL_ADDRESS_MASK);
    pt_level_1->entry[0] = PTE_PRESENT | PTE_WRITABLE | ((Phys_addr)pt_level_2 & PTE_4_KIB_PHYSICAL_ADDRESS_MASK);

    set_cr3((u64)pt_level_1);
}

Phys_addr get_phys_addr(const Page_table* pt_root, const Virt_addr virt_addr)
{
    const Page_table* pt = pt_root;
    Phys_addr phys_addr, frame_base, frame_offset;

    for (u32 level = 1; level <= 4; ++level)
    {
        u64 virt_addr_shift = 48 - 9 * level;
        u64 index = (virt_addr >> virt_addr_shift) & 0b111111111;
        Page_table_entry entry = pt->entry[index];

        if (!(entry & PTE_PRESENT))
            return INVALID_ADDR;

        if (level == 2 && (entry & PTE_HUGE_PAGE))
        {
            frame_base = entry & PTE_1_GIB_PHYSICAL_ADDRESS_MASK;
            frame_offset = virt_addr & 0x3FFFFFFF; // 30 lowest bits

            return frame_base | frame_offset;
        }

        if (level == 3 && (entry & PTE_HUGE_PAGE))
        {
            frame_base = entry & PTE_2_MIB_PHYSICAL_ADDRESS_MASK;
            frame_offset = virt_addr & 0x1FFFFF; // 21 lowest bits

            return frame_base | frame_offset;
        }

        phys_addr = entry & PTE_4_KIB_PHYSICAL_ADDRESS_MASK;
        pt = (const Page_table*)phys_addr;
    }

    frame_base = phys_addr;
    frame_offset = virt_addr & 0xFFF;

    return frame_base | frame_offset;
}
