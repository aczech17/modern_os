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
