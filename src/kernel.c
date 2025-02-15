/*
    VGA colors
    (background << 4) | foreground
    0x0	Black	    0x8	Dark Gray
    0x1	Blue	    0x9	Light Blue
    0x2	Green	    0xa	Light Green
    0x3	Cyan	    0xb	Light Cyan
    0x4	Red	        0xc	Light Red
    0x5	Magenta	    0xd	Pink
    0x6	Brown	    0xe	Yellow
    0x7	Light Gray	0xf	White
*/

#include "vga.h"
#include "memory/memory_map.h"
#include "memory/allocator.h"
#include <stdbool.h>


void print_memory_size(u64 memsize)
{
    const char* units[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB"};
    size_t len = 6;

    for (size_t i = 0; i < len; ++i)
    {
        if ((memsize >> 10) == 0)
        {
            print("%u%s", memsize, units[i]);
            break;
        }

        memsize = memsize >> 10;
    }
}

void kernel_main(u64 mmap_addr, u32 mmap_count, u64 ph_addr, u16 ph_count)
{
    clear_screen(0x07);
    print("%ZSuper system kurwo!\n\n%z", 0x4E);
    
    Memory_map memory_regions;
    set_memory_map(&memory_regions, mmap_addr, mmap_count, 1 << 20);

    u64 total_memory_available = 0;
    print("Available memory sections:\n");
    for (u32 i = 0; i < memory_regions.region_count; ++i)
    {
        total_memory_available += (memory_regions.end_addr[i] - memory_regions.start_addr[i] + 1);
        print("start = %X, end = %X\n", memory_regions.start_addr[i], memory_regions.end_addr[i]);
    }
    u64 total_frames_available = total_memory_available / FRAME_SIZE;
    print("%ZTotal memory available: %X\n", 0x0A, total_memory_available);
    print("Total frames available: %X\n%z", total_frames_available);

    Memory_map kernel_regions;
    kernel_regions.region_count = 0;
    
    print("\nKernel sections:\n");
    for (u16 i = 0; i < ph_count; ++i)
    {
        u64 v_addr = *(u64*)(ph_addr + i * 0x38 + 0x10);
        u64 p_memsz = *(u64*)(ph_addr + i * 0x38 + 0x28);
        u64 end = v_addr + p_memsz - 1;
        print("start = %X, end = %X\n", v_addr, end);

        kernel_regions.start_addr[i] = v_addr;
        kernel_regions.end_addr[i] = end;
        ++kernel_regions.region_count;
    }

    Memory_allocator memory_allocator;
    init_memory_allocator(&memory_allocator, &memory_regions, &kernel_regions);

    u64 free_frames = 0;
    for (u64 block_index = 0; block_index < FRAME_BITMAP_SIZE; ++block_index)
    {
        u8 block = memory_allocator.frame_bitmap[block_index];

        for (u64 i = 0; i < 8; ++i)
        {
            if ((block & 1) == 0)
                ++free_frames;

            block >>= 1;
        }
    }

    print("%ZFree frames:%X\n%z", 0x0A, free_frames);
    
    
    for (;;);
}
