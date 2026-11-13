#ifndef MEM_ALLOCATOR_H
#define MEM_ALLOCATOR_H

#include "phys_memory_map.h"
#include "common.h"

/*
    max memory size = 32GiB
    max frame count = 32GiB/4KiB = 8Mi
    8Mi frames -> 8Mi entries -> 8Mib -> 1MiB
*/
#define FRAME_BITMAP_SIZE (1 << 20)

typedef struct
{
    u8 accessible[FRAME_BITMAP_SIZE];
    u8 kernel_space[FRAME_BITMAP_SIZE];
    u8 taken[FRAME_BITMAP_SIZE];

    u32 latest_allocated_kernel_block;
    u32 latest_allocated_general_block;
}Frame_allocator;

extern Frame_allocator global_frame_allocator;

void setup_frame_allocator
(
    Frame_allocator* allocator,
    Phys_memory_map* available_regions,
    Phys_memory_map* kernel_regions,
    u64 kernel_memory_size
);
Frame_address allocate_kernel_frame(Frame_allocator* allocator);
Frame_address allocate_general_frame(Frame_allocator* allocator);
void deallocate_frame(Frame_allocator* allocator, Frame_address frame_address);

#endif // MEM_ALLOCATOR_H
