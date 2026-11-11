#include "phys_memory_map.h"
#include "../vga.h"

void determine_available_phys_addresses(Phys_memory_map* mmap, const Kernel_mem_info* kernel_mem_info, const Phys_addr low_mem_size)
{
    mmap->region_count = 0;

    u32 mmap_count = kernel_mem_info->mem_map_count;
    Phys_addr mmap_addr = kernel_mem_info->mem_map_address;

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

void register_kernel_regions(Phys_memory_map* kernel_regions, const Kernel_mem_info* kernel_mem_info)
{
    u16 ph_count = kernel_mem_info->ph_entry_count;
    u64 ph_addr = kernel_mem_info->ph_address;

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

    kernel_regions->start_addr[kernel_regions->region_count] = kernel_mem_info->stack_limit;
    kernel_regions->end_addr[kernel_regions->region_count] = kernel_mem_info->stack_base;
    ++kernel_regions->region_count;

    kernel_regions->start_addr[kernel_regions->region_count] = (Phys_addr)VGA_ADDRESS;
    kernel_regions->end_addr[kernel_regions->region_count] = (Phys_addr)(VGA_ADDRESS + VGA_SIZE - 1);
    ++kernel_regions->region_count;

    kernel_regions->start_addr[kernel_regions->region_count] = kernel_mem_info->gdt_address;
    kernel_regions->end_addr[kernel_regions->region_count] = kernel_mem_info->gdt_address + kernel_mem_info->gdt_size - 1;
    ++kernel_regions->region_count;
}
