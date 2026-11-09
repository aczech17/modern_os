#ifndef MEM_ALLOCATOR_H
#define MEM_ALLOCATOR_H

#include "phys_memory_map.h"
#include "common.h"


typedef struct
{
    Phys_memory_map* available_regions;
    Phys_memory_map* kernel_regions;
    u8 frame_bitmap[FRAME_BITMAP_SIZE];
    u32 latest_allocated_block;
}Frame_allocator;

void setup_frame_allocator(Frame_allocator* allocator, Phys_memory_map* available_regions, Phys_memory_map* kernel_regions);
Frame_address allocate_frame(Frame_allocator* allocator);
void deallocate_frame(Frame_allocator* allocator, Frame_address frame_address);

#endif // MEM_ALLOCATOR_H
