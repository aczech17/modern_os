#ifndef PHYS_MEMORY_MAP_H
#define PHYS_MEMORY_MAP_H

#include "../common.h"
#include "common.h"

#define MAX_MEMORY_SECTIONS 128

typedef struct
{
    /*
     memory map address            (u64)
     memory map count              (u32)
     program header address        (u64)
     program header entry count    (u16)
     stack base                    (u64)
     stack limit                   (u64)
     gdt address				   (u64)
     gdt size					   (u16)
*/

    // We sort it by size descending to reduce padding.

    u64 mem_map_address;
    u64 ph_address;
    u64 stack_base;
    u64 stack_limit;
    u64 gdt_address;

    u32 mem_map_count;

    u16 ph_entry_count;
    u16 gdt_size;
}Kernel_mem_info;

typedef struct
{
    Phys_addr start_addr[MAX_MEMORY_SECTIONS];
    Phys_addr end_addr[MAX_MEMORY_SECTIONS];
    u64 region_count;
}Phys_memory_map;

void determine_available_phys_addresses(Phys_memory_map* mmap, const Kernel_mem_info *kernel_mem_info, Phys_addr low_mem_size);
void register_kernel_regions(Phys_memory_map* kernel_regions, const Kernel_mem_info *kernel_mem_info);

#endif // PHYS_MEMORY_MAP_H
