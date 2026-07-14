#include "phys_memory_map.h"

void init_phys_memory_map(Phys_memory_map* mmap, Phys_addr mmap_addr, u32 mmap_count, Phys_addr low_mem_size)
{
    mmap->region_count = 0;

    for (u32 i = 0; i < mmap_count; ++i)
    {
        Phys_addr base = *(u64*)(mmap_addr + i * 24);
        Phys_addr size = *(u64*)(mmap_addr + i * 24 + 8);
        u32 type = *(u32*)(mmap_addr + i * 24 + 16);
        Phys_addr end = base + size - 1;

        if (type != 1 || base < low_mem_size)
            continue;
        
        mmap->start_addr[mmap->region_count] = base;
        mmap->end_addr[mmap->region_count] = end;
        ++mmap->region_count;
    }
}

void init_kernel_regions(Phys_memory_map* kernel_regions, Phys_addr ph_addr, u16 ph_count,
    Phys_addr stack_bottom, Phys_addr stack_top)
{
    kernel_regions->region_count = 0;
    for (u16 i = 0; i < ph_count; ++i)
    {
        u64 v_addr = *(u64*)(ph_addr + i * 0x38 + 0x10);
        u64 p_memsz = *(u64*)(ph_addr + i * 0x38 + 0x28);
        u64 end = v_addr + p_memsz - 1;

        kernel_regions->start_addr[i] = v_addr;
        kernel_regions->end_addr[i] = end;
        ++kernel_regions->region_count;
    }

    kernel_regions->start_addr[kernel_regions->region_count] = stack_bottom;
    kernel_regions->end_addr[kernel_regions->region_count] = stack_top;
    ++kernel_regions->region_count;
}
