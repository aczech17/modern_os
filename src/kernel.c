#include "vga.h"

void kernel_main(u64 kernel_address)
{
    clear_screen(0x07);

    print("Super system kurwo!\n");
    print("%zBrawo, Robercik!\n", 0x14);
    print("%zHej, Jaga gol!\n", 0x4E);

    print("Kernel jest pod adresem %X, czyli po ludzku %d.\n", kernel_address, kernel_address);

    u64 number = 0xdeadbeef;
    print("Test pisania liczb: %x = %X\n", number, number);

    for (;;);
}
