#ifndef MEM_ALLOCATOR_H
#define MEM_ALLOCATOR_H

#include "memory_map.h"
#include "common.h"

/*
    max RAM size = 32 GiB
    frame size = 4 KiB
    max frame count = 32 GiB / 4 KiB = 8 Mi frames
    8 Mi frames -> 8 Mi bools -> 8 Mib = 1 MiB
*/

typedef struct
{
    Memory_map* available_regions;
    Memory_map* kernel_regions;
    u8 frame_bitmap[FRAME_BITMAP_SIZE];
    u32 latest_allocated_block;
}Frame_allocator;

void init_frame_allocator(Frame_allocator* allocator, Memory_map* available_regions, Memory_map* kernel_regions);
u32 allocate_frame(Frame_allocator* allocator);
void deallocate_frame(Frame_allocator* allocator, u32 frame_number);

#endif // MEM_ALLOCATOR_H
