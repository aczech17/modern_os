#include "vga.h"
#include <stdbool.h>

void kernel_main(u64 ph_addr, u16 ph_count, u64 mmap_addr, u32 mmap_count)
{
    clear_screen(0x07);
    print("%zSuper system kurwo!\n\n", 0x4E);

    
    print("Dostepne sekcje pamieci:\n");
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
    }

    print("\nKernel zajmuje sekcje:\n");
    for (u16 i = 0; i < ph_count; ++i)
    {
        u32 p_type = *(u32*)(ph_addr + i * 0x38);
        if (p_type != 1)
            continue;

        u64 v_addr = *(u64*)(ph_addr + i * 0x38 + 0x10);
        u64 p_memsz = *(u64*)(ph_addr + i * 0x38 + 0x28);
        u64 end = v_addr + p_memsz - 1;
        print("start = %X, end = %X\n", v_addr, end);

    }
    
    for (;;);
}
