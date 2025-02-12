#include "vga.h"

void kernel_main(u64 ph_addr, u16 ph_count)
{
    clear_screen(0x07);
    print("%zSuper system kurwo!\n", 0x4E);

    print("ph address = %u\n", (u64)ph_addr);
    print("ph count = %u\n", ph_count);

    //u32 first_entry_p_type = *(u32*)ph_addr;

    for (u16 i = 0; i < ph_count; ++i)
    {
        u32 p_type = *(u32*)(ph_addr + i * 0x38);
        u64 v_addr = *(u64*)(ph_addr + i * 0x38 + 0x10);
        u64 p_memsz = *(u64*)(ph_addr + i * 0x38 + 0x28);
        print("i = %u: p_type = %X, v_addr = %X, p_memsz = %X\n", i, p_type, v_addr, p_memsz);
    }
    
    for (;;);
}
