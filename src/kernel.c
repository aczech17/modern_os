#include <stddef.h>
#include <stdbool.h>
#include "common.h"
#include "vga.h"

void kernel_main(u64 kernel_address)
{
    Vga_buffer vga_buffer =
    {
        .row = 0,
        .col = 0,
        .color = 0x07, // gray on black
    };

    clear_screen(&vga_buffer);
    write_string(&vga_buffer, "Super system kurwo!\n");

    vga_buffer.color = 0x14;
    write_string(&vga_buffer, "Brawo, Robercik!\n");

    vga_buffer.color = 0x4e;
    write_string(&vga_buffer, "Hej, Jaga gol!\n");

    vga_buffer.color = 0x07;
    write_string(&vga_buffer, "Kernel jest pod adresem ");
    write_hex(&vga_buffer, kernel_address, false);

    write_string(&vga_buffer, ",\nczyli po ludzku ");
    write_dec(&vga_buffer, kernel_address);
    write_string(&vga_buffer, ".\n");
    
    for (;;);
}
