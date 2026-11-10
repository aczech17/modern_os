#include "frame_allocator.h"
#include "common.h"
#include "../common.h"

/**
 *
 * Determine which frames are free. Free frames are those available and not belonging to the kernel.
 **/
void setup_frame_allocator(Frame_allocator* allocator, Phys_memory_map* available_regions, Phys_memory_map* kernel_regions)
{
    allocator->available_regions = available_regions;
    allocator->kernel_regions = kernel_regions;
    allocator->latest_allocated_block = 0;

    // Initially mark all frames as used.
    memset(allocator->frame_bitmap, 0xFF, FRAME_BITMAP_SIZE);
    
    // Mark available memory regions as unused.
    for (u32 region = 0; region < available_regions->region_count; ++region)
    {
        Phys_addr region_start = available_regions->start_addr[region];
        Phys_addr region_end   = available_regions->end_addr[region];

        u64 first_frame = (region_start + FRAME_SIZE - 1) / FRAME_SIZE; // Round up
        u64 last_frame = (region_end - FRAME_SIZE + 1) / FRAME_SIZE; // Round down

        for (u64 frame_number = first_frame; frame_number <= last_frame; ++frame_number)
        {
            u64 byte_number = frame_number / 8;
            u64 bit_number = frame_number % 8;
            
            allocator->frame_bitmap[byte_number] &= ~(1 << (7 - bit_number)); // mark as unused
        }
    }

    // Mark kernel frames as used.
    for (u32 region = 0; region < kernel_regions->region_count; ++region)
    {
        Phys_addr region_start = kernel_regions->start_addr[region];
        Phys_addr region_end   = kernel_regions->end_addr[region];

        u64 first_frame = region_start / FRAME_SIZE;
        u64 last_frame = region_end / FRAME_SIZE;

        // It's not so efficient.
        for (u64 frame_number = first_frame; frame_number <= last_frame; ++frame_number)
        {
            u64 byte_number = frame_number / 8;
            u64 bit_number = frame_number % 8;
            
            allocator->frame_bitmap[byte_number] |= (1 << (7 - bit_number));
        }
    }
}

Frame_address allocate_frame(Frame_allocator* allocator)
{
    /*
        1B = 8b ==> frame block = 8 frames
    */
    u64 block_index = allocator->latest_allocated_block;

    for (u32 attempt = 0; attempt < FRAME_BITMAP_SIZE; ++attempt)
    {
        u8* frame_block_bitmap = &allocator->frame_bitmap[block_index];
        if (*frame_block_bitmap == 0xFF) // If the block is full, switch to the next block and try again.
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
            if ((*frame_block_bitmap & mask) == 0)
            {
                *frame_block_bitmap |= mask;   // mark as used
                u64 frame_number = 8 * block_index + i;

                Frame_address frame_address = frame_number * FRAME_SIZE;
                return frame_address;
            }
        }

        block_index = (block_index + 1) % FRAME_BITMAP_SIZE;
    }

    return INVALID_ADDR;
}

void deallocate_frame(Frame_allocator* allocator, Frame_address frame_address)
{
    u64 frame_number = frame_address / FRAME_SIZE;
    u64 frame_block = frame_number / 8;
    u64 frame_in_block = frame_number % 8;
    allocator->frame_bitmap[frame_block] &= ~(1 << (7 - frame_in_block));
}
