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

bool map_address(Virt_addr virt_addr, Phys_addr phys_addr)
{
    Page_table* current_page_table = global_page_table;
    for (u32 level = 1; level <= 4; ++level)
    {
        u64 virt_addr_shift = 48 - 9 * level;
        u64 index = (virt_addr >> virt_addr_shift) & 0b111111111;
        Page_table_entry entry = current_page_table->entry[index];

        if (level == 4)
        {
            current_page_table->entry[index] = PTE_PRESENT | PTE_WRITABLE | (phys_addr & PTE_4_KIB_PHYSICAL_ADDRESS_MASK);
            invalidate_tlb_entry(virt_addr);
            return true;
        }

        if (!(entry & PTE_PRESENT))
        {
            Frame_address new_frame = allocate_kernel_frame(&global_frame_allocator);
            if (new_frame == INVALID_ADDR)
                return false;

            Page_table* next_lvl_pt_addr = (Page_table*)new_frame;
            memset(next_lvl_pt_addr, 0, FRAME_SIZE);

            current_page_table->entry[index] = PTE_PRESENT | PTE_WRITABLE |
                ((Phys_addr)next_lvl_pt_addr & PTE_4_KIB_PHYSICAL_ADDRESS_MASK);
        }

        current_page_table = (Page_table*)(current_page_table->entry[index] & PTE_4_KIB_PHYSICAL_ADDRESS_MASK);
    }

    return true;
}

Phys_addr get_phys_addr(const Virt_addr virt_addr)
{
    const Page_table* pt = global_page_table;
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
