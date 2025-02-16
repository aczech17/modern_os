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

void set_memory_map(Memory_map* mmap, u64 mmap_addr, u32 mmap_count, u64 low_mem_size);

#endif // MEMORY_MAP_H
