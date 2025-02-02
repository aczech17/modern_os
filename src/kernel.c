#include <stddef.h>
#include <stdbool.h>
#include "common.h"
#include "vga.h"

void kernel_main(u64 kernel_address)
{
    clear_screen(0x07);

    const char* welcome_message = "Super system kurwo!";
    print_formatted("%s\nKernel jest pod adresem %x,\nczyli po ludzku %d.\n\n",
                    welcome_message, kernel_address, kernel_address);

    u64 number = 0xdeadbeef;
    print_formatted("%x to inaczej %X\n", number, number);

    for (;;);
}
