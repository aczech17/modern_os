#include "frame_allocator.h"
#include "common.h"
#include "../common.h"

void init_frame_allocator(Frame_allocator* allocator, Memory_map* available_regions, Memory_map* kernel_regions)
{
    allocator->available_regions = available_regions;
    allocator->kernel_regions = kernel_regions;
    allocator->latest_allocated_block = 0;

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

u32 allocate_frame(Frame_allocator* allocator)
{
    u32 block_index = allocator->latest_allocated_block;

    for (u32 attempt = 0; attempt < FRAME_BITMAP_SIZE; ++attempt)
    {
        u8* frame_block = &allocator->frame_bitmap[block_index];
        if (*frame_block == 0xFF) // If the block is full, switch to the next block and try again.
        {
            block_index = (block_index + 1) % FRAME_BITMAP_SIZE;
            continue;
        }
        
        // We found a non-full frame block.
        allocator->latest_allocated_block = block_index;

        // Search for a free frame in this block. Surely there is one.
        for (u32 i = 0; i < 8; ++i)
        {
            u8 mask = 0x80 >> i;
            if ((*frame_block & mask) == 0)
            {
                *frame_block |= mask;   // mark as used
                u32 frame_number = 8 * block_index + i;
                return frame_number;
            }
        }

        block_index = (block_index + 1) % FRAME_BITMAP_SIZE;
    }

    return INVALID_ADDR;
}

void deallocate_frame(Frame_allocator* allocator, u32 frame_number)
{
    u32 frame_block = frame_number / 8;
    u32 frame_in_block = frame_number % 8;
    allocator->frame_bitmap[frame_block] &= ~(1 << (7 - frame_in_block));
}
