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
#include "memory/phys_memory_map.h"
#include "memory/frame_allocator.h"
#include "memory/page_table.h"
#include "interrupt.h"

static Idt_entry idt[256];

static void print_memory_map(const Phys_memory_map* memory_regions, const char* name)
{
    vga_printf("%Z%s%z:\n", 0x09 ,name);
    u64 total_memory_available = 0;
    for (u32 i = 0; i < memory_regions->region_count; ++i)
    {
        total_memory_available += (memory_regions->end_addr[i] - memory_regions->start_addr[i] + 1);
        vga_printf("start = %X, end = %X\n", memory_regions->start_addr[i], memory_regions->end_addr[i]);
    }
    u64 total_frames_available = total_memory_available / FRAME_SIZE;
    vga_printf("%ZTotal memory in %s: %X\n", 0x0A, name, total_memory_available);
    vga_printf("Total frames in %s: %X\n%z\n", name, total_frames_available);
}

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

void kernel_main
(
    u64 memory_map_addr,
    u32 memory_map_entry_count,
    u64 kernel_program_header_addr,
    u16 kernel_program_header_entry_count,
    u64 stack_bottom,
    u64 stack_top
)
{
    clear_screen(0x07);
    vga_printf("%ZSuper system!\n%z\n", 0x4E);
    
    Phys_memory_map phys_memory_regions;

    determine_available_phys_addresses(&phys_memory_regions, memory_map_addr, memory_map_entry_count, 1 << 20);

    Phys_memory_map kernel_regions;
    init_kernel_regions(&kernel_regions, kernel_program_header_addr, kernel_program_header_entry_count, stack_bottom, stack_top);

    print_memory_map(&phys_memory_regions, "total available");
    print_memory_map(&kernel_regions, "taken by kernel");

    vga_printf("stack_bottom = %X\n", stack_bottom);
    vga_printf("stack top = %X\n\n", stack_top);

    Frame_allocator frame_allocator;
    init_frame_allocator(&frame_allocator, &phys_memory_regions, &kernel_regions);

    u64 free_frames_count = get_free_frames_count(&frame_allocator);
    vga_printf("%Zfree frames count = %X\n%z", 0x0F, free_frames_count);


    /*
        u8 present;
        u8 writable;
        u8 user_accessible;
        u8 write_through_caching;
        u8 cache_disable;
        u8 accessed;
        u8 dirty;
        u8 huge_page;
        u8 global;
        u64 available;
        u64 phys_addr;
        u64 no_execute;
    */

    alignas (4096) Page_table pt1;
    Page_table pt2;

    Page_table_entry pt1_entry =
    {
        .present = 1,
        .writable = 1,
        .user_accessible = 0,
        .write_through_caching = 0,
        .cache_disable = 0,
        .accessed = 0,
        .dirty = 0,
        .huge_page = 0,
        .global = 0,
        .available = 0,
        .phys_addr = (Phys_addr)&pt2,
        .no_execute = 0,
    };
    pt1.entry[0] = page_table_value(&pt1_entry);

    Page_table_entry pt2_entry =
    {
        .present = 1,
        .writable = 1,
        .user_accessible = 0,
        .write_through_caching = 0,
        .cache_disable = 0,
        .accessed = 0,
        .dirty = 0,
        .huge_page = 1,
        .global = 0,
        .available = 0,
        .phys_addr = 0,
        .no_execute = 0,
    };
    pt2.entry[0] = page_table_value(&pt2_entry);

    __asm__ volatile
    (
        "mov %0, %%cr3\n"
        :: "r"(&pt1)
        : "memory"
    );


    vga_printf("\n%ZPage tables OK :)%z\n", 0x20);

    u64 handler = (u64)page_fault_handler;

    idt[14].offset_low    = handler & 0xFFFF;
    idt[14].offset_middle = (handler >> 16) & 0xFFFF;
    idt[14].offset_high   = (handler >> 32) & 0xFFFFFFFF;

    idt[14].segment_selector = 0x08;
    idt[14].ist = 0;
    idt[14].type_attrs = 0x8E;
    idt[14].reserved = 0;

    u8 idtr[10];

    u16 limit = sizeof(idt) - 1;
    u64 base = (u64)idt;

    idtr[0] = limit;
    idtr[1] = limit >> 8;

    for (u8 i = 0; i < 8; ++i)
        idtr[i + 2] = base >> (8 * i);

    __asm__ volatile
    (
        "lidt %0"
        :
        : "m"(idtr)
    );

    vga_printf("%ZINTs prepared. Trying to provoke pf.%z\n", 0x0D);

    volatile u64* ptr = (u64*)0xDEADBEEF;
    u64 value = *ptr;

    for (;;);
}
