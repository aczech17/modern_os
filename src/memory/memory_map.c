#include "memory_map.h"
#include <stdbool.h>

void init_memory_map(Memory_map* mmap, u64 mmap_addr, u32 mmap_count, u64 low_mem_size)
{
    mmap->region_count = 0;

    for (u32 i = 0; i < mmap_count; ++i)
    {
        u64 base = *(u64*)(mmap_addr + i * 24);
        u64 size = *(u64*)(mmap_addr + i * 24 + 8);
        u32 type = *(u32*)(mmap_addr + i * 24 + 16);
        u64 end = base + size - 1;

        if (type != 1 || base < low_mem_size)
            continue;
        
        mmap->start_addr[mmap->region_count] = base;
        mmap->end_addr[mmap->region_count] = end;
        ++mmap->region_count;
    }
}

void init_kernel_regions(Memory_map* kernel_regions, u64 ph_addr, u16 ph_count, u64 stack_top, u64 stack_bottom)
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

    kernel_regions->start_addr[kernel_regions->region_count] = stack_top;
    kernel_regions->end_addr[kernel_regions->region_count] = stack_bottom;
    ++kernel_regions->region_count;
}
