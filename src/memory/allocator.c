#include "allocator.h"
#include "../common.h"

void init_memory_allocator(Memory_allocator* allocator, Memory_map* available_regions, Memory_map* kernel_regions)
{
    allocator->available_regions = available_regions;
    allocator->kernel_regions = kernel_regions;

    // Initially mark all frames as used.
    memory_set(allocator->frame_bitmap, 0xFF, FRAME_BITMAP_SIZE);
    
    // Mark available memory regions as unused.
    for (u32 region = 0; region < available_regions->region_count; ++region)
    {
        u64 region_start = available_regions->start_addr[region];
        u64 region_end   = available_regions->end_addr[region];
        u64 first_frame = region_start / FRAME_SIZE;
        u64 last_frame = region_end / FRAME_SIZE; // Round down

        for (u64 frame_number = first_frame; frame_number <= last_frame; ++frame_number)
        {
            u64 byte_number = frame_number / 8;
            u64 bit_number = frame_number % 8;
            
            allocator->frame_bitmap[byte_number] &= ~(1 << (7 - bit_number)); // mark as unused
        }
    }

    // Mark kernel frames used.
    for (u32 region = 0; region < kernel_regions->region_count; ++region)
    {
        u64 region_start = kernel_regions->start_addr[region];
        u64 region_end   = kernel_regions->end_addr[region];
        u64 first_frame = region_start / FRAME_SIZE;
        u64 last_frame = (region_end + FRAME_SIZE - 1) / FRAME_SIZE; // Round up.

        // It's not so efficient.
        for (u64 frame_number = first_frame; frame_number <= last_frame; ++frame_number)
        {
            u64 byte_number = frame_number / 8;
            u64 bit_number = frame_number % 8;
            
            allocator->frame_bitmap[byte_number] |= (1 << (7 - bit_number));
        }
    }
}

u32 allocate_frame(Memory_allocator* allocator)
{
    for (u32 block_index = 0; block_index < FRAME_BITMAP_SIZE; ++block_index)
    {
        u8* frame_block = &allocator->frame_bitmap[block_index];
        if (*frame_block == 0xFF)
            continue;

        for (u32 i = 0; i < 8; ++i)
        {
            u8 mask = 0x80 >> i;
            if ((*frame_block & mask) != 0)
            {
                *frame_block |= mask;
                u32 frame_number = 8 * block_index + i;
                return frame_number;
            }
        }
    }

    return 0;
}
