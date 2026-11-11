#include <stdalign.h>
#include "memory/phys_memory_map.h"
#include "memory/frame_allocator.h"
#include "memory/page_table.h"
#include "interrupt/idt.h"

static alignas(4096) Page_table pt_level1;
static alignas(4096) Page_table pt_level2;
static alignas(4096) Page_table pt_level3;
static alignas(4096) Page_table pt_level4[512];

static Idt_entry idt[256];

static void print_memory_map(const Phys_memory_map* memory_regions, const char* name)
{
    // vga_printf("%Z%s:%z\n", 0x09 ,name);
    u64 total_memory_available = 0;
    for (u32 i = 0; i < memory_regions->region_count; ++i)
    {
        total_memory_available += (memory_regions->end_addr[i] - memory_regions->start_addr[i] + 1);
        // vga_printf("start = %X, end = %X\n", memory_regions->start_addr[i], memory_regions->end_addr[i]);
    }
    // u64 total_frames_available = total_memory_available / FRAME_SIZE;

    vga_printf("Total memory in %s: %XB\n", name, total_memory_available);
    // vga_printf("Total frames in %s: %X\n%z\n", name, total_frames_available);
}

/*
static u64 get_free_frames_count(const Frame_allocator *frame_allocator)
{
    u64 free_frames_count = 0;
    for (u64 block_index = 0; block_index < FRAME_BITMAP_SIZE; ++block_index)
    {
        u8 block = frame_allocator->frame_bitmap[block_index];

        for (u64 i = 0; i < 8; ++i)
        {
            if ((block & 1) == 0)
                ++free_frames_count;

            block >>= 1;
        }
    }

    return free_frames_count;
}
*/

void kernel_main(const Kernel_mem_info* kernel_mem_info)
{
    const Phys_addr LOW_MEM_SIZE = 1 << 20;

    disable_bios_cursor();
    vga_printf("%a"); // clear screen
    vga_printf("%ZModern OS. Witamy!\n%z\n", 0x1F);

    Phys_memory_map phys_memory_regions;
    determine_available_phys_addresses(&phys_memory_regions, kernel_mem_info, LOW_MEM_SIZE);

    Phys_memory_map kernel_regions;
    register_kernel_regions(&kernel_regions, kernel_mem_info);

    print_memory_map(&phys_memory_regions, "available memory");
    print_memory_map(&kernel_regions, "kernel memory");

    // vga_printf("kernel stack base = %X\n", kernel_mem_info->stack_base);
    // vga_printf("kernel stack limit = %X\n\n", kernel_mem_info->stack_limit);

    Frame_allocator frame_allocator;
    setup_frame_allocator(&frame_allocator, &phys_memory_regions, &kernel_regions);

    // u64 free_frames_count = get_free_frames_count(&frame_allocator);
    // vga_printf("%Zfree frames count = %X\n%z", 0x0F, free_frames_count);

    setup_page_tables(&pt_level1, &pt_level2, &pt_level3, pt_level4, &kernel_regions);
    vga_printf("Page tables set up.\n");

    setup_idt(idt);
    vga_printf("IDT set up.%z\n");

    vga_printf("\nTrying to provoke page fault.\n");
    volatile u8 val = *(u8*)(1 << 30);

    for (;;);
}
