#include <stddef.h>
#include <stdbool.h>
#include "common.h"
#include "vga.h"

size_t strlen(const char* text)
{
    size_t len = 0;
    for (; *text != 0; ++text)
        ++len;
    return len;
}

void kernel_main(u64 kernel_address)
{
    Vga_buffer vga_buffer =
    {
        .row = 0,
        .col = 0,
        .color = 0x07, // gray on black
    };

    clear_screen(&vga_buffer);
    write_string(&vga_buffer, "Booting Modern OS...\n");

    const char* kernel_location_msg = "Kernel loaded to address: ";
    write_string(&vga_buffer, kernel_location_msg);
    write_hex(&vga_buffer, kernel_address, false);

    write_string(&vga_buffer, "\nTEST");
    for (u64 i = 0; ; ++i)
    {
        write_string(&vga_buffer, "\n");
        write_hex(&vga_buffer, i, true);
    }

    for (;;);
}
