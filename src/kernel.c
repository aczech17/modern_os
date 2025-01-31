#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "vga.h"

size_t strlen(const char* text)
{
    size_t len = 0;
    for (; *text != 0; ++text)
        ++len;
    return len;
}

void kernel_main(uint64_t kernel_address)
{
    clear_vga();
    write_string_vga("Booting Modern OS...", 0, 0);

    const char* kernel_location_msg = "Kernel loaded to address: ";
    write_string_vga(kernel_location_msg, 1, 0);

    write_hex(kernel_address, 1, strlen(kernel_location_msg), true);

    for (;;);
}
