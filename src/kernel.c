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
    
    u64 total_memory_available = 0;
    print("Physical memory sections:\n");
    for (u32 i = 0; i < mmap_count; ++i)
    {
        u64 base = *(u64*)(mmap_addr + i * 24);
        u64 size = *(u64*)(mmap_addr + i * 24 + 8);
        u32 type = *(u32*)(mmap_addr + i * 24 + 16);
        u32 attr = *(u32*)(mmap_addr + i * 24 + 20);

        bool ignored = (attr & 1) == 0;
        if (ignored || type != 1)
            continue;

        u64 end = base + size - 1;
        print("base = %X, end = %X\n", base, end);

        if (base > 0)
            total_memory_available += size;
    }
    print("%ZTotal memory available: %XB ~= ", 0x0A, total_memory_available);
    print_memory_size(total_memory_available);
    print("%z\n");

    // print("%u\n", total_memory_available);

    u64 frame_memory_available = total_memory_available;
    print("\nKernel sections:\n");
    for (u16 i = 0; i < ph_count; ++i)
    {
        u32 p_type = *(u32*)(ph_addr + i * 0x38);
        if (p_type != 1)
            continue;

        u64 v_addr = *(u64*)(ph_addr + i * 0x38 + 0x10);
        u64 p_memsz = *(u64*)(ph_addr + i * 0x38 + 0x28);
        u64 end = v_addr + p_memsz - 1;
        print("start = %X, end = %X\n", v_addr, end);

        frame_memory_available -= p_memsz;
    }
    print("%ZMemory available for allocation: %XB ~= ", 0x0A, frame_memory_available);
    print_memory_size(frame_memory_available);
    print("%z\n");
    
    for (;;);
}
