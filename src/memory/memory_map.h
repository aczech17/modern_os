#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include "../common.h"

#define MAX_MEMORY_SECTIONS 128

typedef struct
{
    u64 start_addr[MAX_MEMORY_SECTIONS];
    u64 end_addr[MAX_MEMORY_SECTIONS];
    u64 region_count;
}Memory_map;

void init_memory_map(Memory_map* mmap, u64 mmap_addr, u32 mmap_count, u64 low_mem_size);
void init_kernel_regions(Memory_map* kernel_regions, u64 ph_addr, u16 ph_count, u64 stack_top, u64 stack_bottom);

#endif // MEMORY_MAP_H
