#include "vga.h"

void kernel_main(u64 ph_addr, u16 ph_count, u64 mmap_addr, u32 mmap_count)
{
    clear_screen(0x07);
    print("%zSuper system kurwo!\n", 0x4E);

    print("ph address = %u\n", (u64)ph_addr);
    print("ph count = %u\n", ph_count);

    //u32 first_entry_p_type = *(u32*)ph_addr;

    u64 total_memory_taken = 0;
    for (u16 i = 0; i < ph_count; ++i)
    {
        u32 p_type = *(u32*)(ph_addr + i * 0x38);
        u64 v_addr = *(u64*)(ph_addr + i * 0x38 + 0x10);
        u64 p_memsz = *(u64*)(ph_addr + i * 0x38 + 0x28);
        print("i = %u: p_type = %X, v_addr = %X, p_memsz = %X\n", i, p_type, v_addr, p_memsz);

        total_memory_taken += p_memsz;
    }

    print("Zajete %uB\n", total_memory_taken);

    print("mmap_count = %u\n", mmap_count);
    for (u32 i = 0; i < mmap_count; ++i)
    {
        // 0xB8000
        u64 base = *(u64*)(mmap_addr + i * 24);
        u64 size = *(u64*)(mmap_addr + i * 24 + 8);
        u32 type = *(u32*)(mmap_addr + i * 24 + 16);
        u32 attr = *(u32*)(mmap_addr + i * 24 + 20);
        u64 end = base + size - 1;


        print("base = %X, end = %X, type = %X\n", base, end, type);
        print("attr = %X\n\n", attr);

        if (0xB8000 >= base && 0xB8000 <= end)
            print("VGA\n");

    }
    
    for (;;);
}
