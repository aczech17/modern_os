#include "allocator.h"

/*
    max RAM size = 32 GiB
    frame size = 4 KiB
    max frame count = 32 GiB / 4 KiB = 8 Mi frames
    8 Mi frames -> 8 Mi bools -> 8 Mib = 1 MiB
*/
#define FRAME_BITMAP_SIZE 1 << 20 // 1 MiB

struct Memory_allocator
{
    Memory_map* available_sections;
    Memory_map* kernel_sections;
    char frame_bitmap[FRAME_BITMAP_SIZE];
};

void init_memory_allocator
    (struct Memory_allocator* allocator, Memory_map* available_sections, Memory_map* kernel_sections)
{
    allocator->available_sections = available_sections;
    allocator->kernel_sections = kernel_sections;
    for (u32 i = 0; i < FRAME_BITMAP_SIZE; ++i)
    {
        allocator->frame_bitmap[i] = 0; // Mark all frames unused.
    }
}
