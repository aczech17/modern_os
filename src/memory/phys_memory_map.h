#ifndef PHYS_MEMORY_MAP_H
#define PHYS_MEMORY_MAP_H

#include "../common.h"

#define MAX_MEMORY_SECTIONS 128

typedef u64 Phys_addr;

typedef struct
{
    Phys_addr start_addr[MAX_MEMORY_SECTIONS];
    Phys_addr end_addr[MAX_MEMORY_SECTIONS];
    u64 region_count;
}Phys_memory_map;

void init_phys_memory_map(Phys_memory_map* mmap, Phys_addr mmap_addr, u32 mmap_count, Phys_addr low_mem_size);
void init_kernel_regions(Phys_memory_map* kernel_regions, Phys_addr ph_addr, u16 ph_count, Phys_addr stack_top, Phys_addr stack_bottom);

#endif // PHYS_MEMORY_MAP_H
