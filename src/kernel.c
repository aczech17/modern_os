#include <stddef.h>
#include <stdbool.h>
#include "common.h"
#include "vga.h"

void kernel_main(u64 kernel_address)
{
    clear_screen(0x07);

    print_formatted("Super system kurwo!\n");
    print_formatted("%zBrawo, Robercik!\n", 0x14);
    print_formatted("%zHej, Jaga gol!\n", 0x4E);

    print_formatted("Kernel jest pod adresem %X, czyli po ludzku %d.\n", kernel_address, kernel_address);

    u64 number = 0xdeadbeef;
    print_formatted("Test pisania liczb: %x = %X\n", number, number);

    for (;;);
}
