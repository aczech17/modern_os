#include "allocator.h"
#include "../common.h"

void init_memory_allocator(Memory_allocator* allocator, Memory_map* available_sections, Memory_map* kernel_sections)
{
    allocator->available_sections = available_sections;
    allocator->kernel_sections = kernel_sections;

    // Initially mark all frames unused.
    memory_set(allocator->frame_bitmap, 0, FRAME_BITMAP_SIZE);
    
    // Mark kernel frames used.
    for (u32 section = 0; section < kernel_sections->section_count; ++section)
    {
        u64 section_start = kernel_sections->start_addr[section];
        u64 section_end   = kernel_sections->end_addr[section];
        u64 first_frame = section_start / FRAME_SIZE;
        u64 last_frame = (section_end + FRAME_SIZE - 1) / FRAME_SIZE; // Round up.

        // It's not so efficient.
        for (u64 frame_number = first_frame; frame_number <= last_frame; ++frame_number)
        {
            u64 byte_number = frame_number / 8;
            u64 bit_number = frame_number % 8;
            
            allocator->frame_bitmap[byte_number] |= (1 << (7 - bit_number));
        }
    }
}
