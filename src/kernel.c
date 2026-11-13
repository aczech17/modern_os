#include <stdalign.h>
#include "memory/phys_memory_map.h"
#include "memory/frame_allocator.h"
#include "memory/page_table.h"
#include "interrupt/idt.h"

Page_table* page_table;
static Idt_entry idt[256];
Frame_allocator global_frame_allocator;

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

    vga_printf("%s: %X bytes\n", name, total_memory_available);
    // vga_printf("Total frames in %s: %X\n%z\n", name, total_frames_available);
}

void kernel_main(const Kernel_mem_info* kernel_mem_info)
{
    const Phys_addr LOW_MEM_SIZE = 1 << 20;
    const u64 KERNEL_MEM_SIZE = 1 << 30; // The government will feed itself.

    disable_bios_cursor();
    vga_printf("%a"); // clear screen
    vga_printf("%ZModern OS. Witamy!\n%z\n", 0x1F);

    Phys_memory_map phys_memory_regions;
    determine_available_phys_addresses(&phys_memory_regions, kernel_mem_info, LOW_MEM_SIZE);

    Phys_memory_map kernel_regions;
    register_kernel_regions(&kernel_regions, kernel_mem_info);

    print_memory_map(&phys_memory_regions, "available memory");
    print_memory_map(&kernel_regions, "kernel memory");


    setup_frame_allocator(&global_frame_allocator, &phys_memory_regions, &kernel_regions, KERNEL_MEM_SIZE);

    page_table = (Page_table*)(kernel_mem_info->page_table_address);
    vga_printf("\nPage table at %X\n", page_table);

    // setup_page_tables(&pt_level1, &pt_level2, &pt_level3, pt_level4, &kernel_regions);
    // vga_printf("Page tables set up.\n");

    setup_idt(idt);
    vga_printf("IDT set up.%z\n");

    // vga_printf("Frame allocation test:\n");
    // for (int i = 0; i < 5; ++i)
    // {
    //     Frame_address kernel_frame = allocate_kernel_frame(&global_frame_allocator);
    //     Frame_address general_frame = allocate_general_frame(&global_frame_allocator);
    //
    //     vga_printf("K: %X; G: %X\n", kernel_frame, general_frame);
    // }

    vga_printf("\nTrying to provoke page fault.\n");
    volatile u8* addr = (u8*)(1 << 31);
    *addr = 0x37;

    vga_printf("%Z\nval=%X\nOK%z\n", 0x2F, *addr);

    for (;;);
}
